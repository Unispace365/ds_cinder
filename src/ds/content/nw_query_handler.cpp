#include "stdafx.h"

#include "nw_query_handler.h"

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/content/content_events.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/content/content_events.h>
#include <ds/util/boolinq.h>

#define CN_BUFSIZE 256

namespace ds::model {

NWQueryHandler::NWQueryHandler(ds::ui::SpriteEngine &eng)
    : mEngine(eng), mEventClient(eng) {

  // make a buffer for the computer name
  DWORD bufsize = CN_BUFSIZE;
  char computername_buffer[CN_BUFSIZE];
  memset(computername_buffer, 0, CN_BUFSIZE);

  // get the forrealz computer name.
  GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsHostname,
                     computername_buffer, &bufsize);

  
  auto computerName = mEngine.getAppSettings().getString(
      "platform:override_computername", 0, computername_buffer);
  mPlatformKey = mEngine.getAppSettings().getString(
	  "platform:key", 0, computerName);
  auto platformKeyUpper = mPlatformKey;
  ds::to_uppercase(platformKeyUpper);
  if ((mPlatformKey.empty() ||  platformKeyUpper == "AUTO") && !computerName.empty()) {
	  mPlatformKey = computerName;
  }
  DS_LOG_INFO("Discovered Platform Key of: " << mPlatformKey);
  mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](auto& e) {
	  handleQuery();
  });
}

void NWQueryHandler::handleQuery() {

  DS_LOG_INFO("GETTING NEW DATA: ");
  int drawingParent = 0;

  // tags
  ds::model::ContentModelRef tags = ds::model::ContentModelRef("cms_tags");
  mEngine.mContent.replaceChild(tags);

  // create node list
  
  std::vector<ds::model::ContentModelRef> rawNodesList =
      mEngine.mContent.getChildByName("sqlite.waffles_nodes").getChildren();

  std::string unflitered_root_name = "cms_unfiltered_root";
  std::map<int, ds::model::ContentModelRef> allUFNodes;
  std::vector<ds::model::ContentModelRef> rawUFNodesList;

  // create unfiltered node copy
  ds::model::ContentModelRef cmsUFRoot =
      ds::model::ContentModelRef(unflitered_root_name);
  for (auto it : rawNodesList) {
    rawUFNodesList.push_back(it.duplicate());
  }

  // populate the unfiltered tree
  for (auto &it : rawUFNodesList) {

    parseModelProperties(it, rawUFNodesList);

    auto theType = it.getPropertyString("type");
    auto theBranch = it.getPropertyString("branch");
    if (theType == "media" && it.getPropertyResource("media_res").empty()) {
      DS_LOG_WARNING("No media detected on raw node "
                     << it.getId() << " " << it.getPropertyString("name"));
      continue;
    }
    if (theBranch == "content") {
      allUFNodes[it.getId()] = it;
    }
  }

  // build unfiltered tree
  for (auto &it : rawUFNodesList) {
    auto theBranch = it.getPropertyString("branch");
    if (theBranch != "content")
      continue;

    auto parentId = it.getPropertyInt("parent_id");
    if (parentId == 0) {
      cmsUFRoot.addChild(it);
    } else {
      // if parentId does not exist as a key
      //(in the instance the parent is disabled and the children arent)
      // this adds a blank contentModelRef which gets deleted later
      allUFNodes[parentId].addChild(it);
    }
  }
  mEngine.mContent.replaceChild(cmsUFRoot);

  //if we were filtering this is where it would be.
  std::vector<std::string> excluded_templates =
      ds::split(mEngine.getAppSettings().getString("kind_exclude", 0, ""),
                ", ", true);
  std::vector<ds::model::ContentModelRef> allNodesList = rawNodesList;
      //boolinq::from(rawNodesList)
      //    .where([excluded_templates,
      //            allUFNodes](ds::model::ContentModelRef it) {
      //      auto kind = it.getPropertyString("kind");
      //      if (std::find(excluded_templates.begin(), excluded_templates.end(),
      //                    kind) != excluded_templates.end()) {
      //        DS_LOG_INFO("excluding " << kind);
      //        return false;
      //      }
      //      if (kind == "template_overview") {
      //        auto rawItem = allUFNodes.at(it.getId());
      //        auto children = rawItem.getPropertyListInt("items");

      //        for (auto ch : children) {

      //          auto chIt = allUFNodes.at(ch);
      //          auto chKind = chIt.getPropertyString("kind");
      //          auto branch = chIt.getPropertyString("branch");
      //          // if any child is allowed return that the parent is allowed.
      //          if (branch == "content" &&
      //              std::find(excluded_templates.begin(),
      //                        excluded_templates.end(),
      //                        chKind) == excluded_templates.end()) {
      //            return true;
      //          }
      //        }
      //        return false;
      //      }
      //      return true;
      //    })
      //    .toStdVector();

  // ----------- PLATFORMS --------------------------------------
  ds::model::ContentModelRef cmsPlatforms =
      ds::model::ContentModelRef("cms_platforms");
  cmsPlatforms.setProperty("kind", std::string("platforms"));
  cmsPlatforms.setProperty("name", std::string("Platforms"));

  auto nodes =
      mEngine.mContent.getChildByName("sqlite.waffles_nodes").getChildren();
  // add properties to the platforms
  std::vector<ds::model::ContentModelRef> allPlatformsList;
  std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(allPlatformsList),
               [](ds::model::ContentModelRef n) {
                 bool result = n.getPropertyString("branch") == "platform";
                 return result;
               });

  /// filter out all platforms to this computer name
  /// Note, you could change mComputerName to something from app settings or
  /// other value to filter in other ways
  ds::model::ContentModelRef thisPlatform;
  int platformCount = 0;
  if (allPlatformsList.empty()) {
	  DS_LOG_WARNING("Did not find any platforms in the database.");
  }

  cmsPlatforms.clearChildren();
  for (auto it : allPlatformsList) {
    parseModelProperties(it, allNodesList);
    if (it.getPropertyString("platform_key") == mPlatformKey) {
      thisPlatform = it;
	  platformCount++;
	  cmsPlatforms.addChild(thisPlatform);
    }
  }

  if (platformCount > 1) {
	  DS_LOG_WARNING("Found Multiple platforms that match the given platform key of " << mPlatformKey
		  << " This is probably not what you want");
  }
  
  if (thisPlatform.empty() && !allPlatformsList.empty()) {
    DS_LOG_WARNING("No specific platform for this app. Check the computer name "
                   << mPlatformKey << " against the CMS");
  }
  else if(!allPlatformsList.empty()) {
	  DS_LOG_INFO("Found matching platform "
		  << mPlatformKey << " against the CMS");
  }

  mEngine.mContent.replaceChild(cmsPlatforms);

  // ----------- NODES --------------------------------------
  // The main waffles nodes, aka the primary content
  ds::model::ContentModelRef cmsRoot = ds::model::ContentModelRef("cms_root");
  cmsRoot.setProperty("kind", std::string("node"));
  cmsRoot.setProperty("name", std::string("Nodes"));

  std::map<int, ds::model::ContentModelRef> allNodes;

  for (auto &it : allNodesList) {
    parseModelProperties(it, allNodesList);

    auto theType = it.getPropertyString("type");
    auto theBranch = it.getPropertyString("branch");
    if (theType == "media" && it.getPropertyResource("media_res").empty()) {
      DS_LOG_WARNING("No media detected on node "
                     << it.getId() << " " << it.getPropertyString("name"));
      continue;
    }
    if (theBranch == "content") {
      allNodes[it.getId()] = it;

      if (it.getPropertyBool("drawings_folder")) {
        drawingParent = it.getId();
      }
    }
  }

  // set up the node tree
  for (auto &it : allNodesList) {
    auto theBranch = it.getPropertyString("branch");
    if (theBranch != "content")
      continue;

    auto parentId = it.getPropertyInt("parent_id");
    if (parentId == 0) {
      cmsRoot.addChild(it);
    } else {
      // if parentId does not exist as a key
      //(in the instance the parent is disabled and the children arent)
      // this adds a blank contentModelRef which gets deleted later
      allNodes[parentId].addChild(it);
    }
  }

  // add valid nodes as references on the root node for lookup speed
  std::map<int, ds::model::ContentModelRef> allValidNodes;
  addReference(cmsRoot, allValidNodes);
  cmsRoot.setReferences("valid_nodes", allValidNodes);

  std::map<int, ds::model::ContentModelRef> allUnfilteredValidNodes;
  addReference(cmsUFRoot, allUnfilteredValidNodes);
  cmsUFRoot.setReferences("valid_nodes", allUnfilteredValidNodes);

  cmsRoot.setProperty("drawings_folder_id", drawingParent);

  // add the new tree of nodes
  mEngine.mContent.replaceChild(cmsRoot);

  // ----------- Events --------------------------------------
  ds::model::ContentModelRef cmsEvents =
      ds::model::ContentModelRef("cms_events");
  cmsEvents.setProperty("kind", std::string("event"));
  cmsEvents.setProperty("name", std::string("Events"));
  int platform_id = thisPlatform.getId();

  auto scheds_x_platform =
      mEngine.mContent.getChildByName("sqlite.wfl_platform_schedule")
          .getChildren();
  std::vector<int> platform_schedules;
  for (auto connection : scheds_x_platform) {
    if (connection.getPropertyInt("node_id") == platform_id) {
      platform_schedules.push_back(connection.getPropertyInt("schedule_id"));
    }
  }
  std::vector<ds::model::ContentModelRef> eventList;
  std::copy_if(rawUFNodesList.begin(), rawUFNodesList.end(),
               std::back_inserter(eventList),
               [platform_schedules](ds::model::ContentModelRef n) {
                 auto branch = n.getPropertyString("branch");
                 bool isEvent = branch == "event";

                 int schedule_id = n.getPropertyInt("schedule_id");
                 bool isOnPlatform =
                     std::find(platform_schedules.begin(),
                               platform_schedules.end(),
                               schedule_id) != platform_schedules.end();
                 return isEvent && isOnPlatform;
               });

  cmsEvents.setChildren(eventList);
  mEngine.mContent.replaceChild(cmsEvents);

  //---AND LASTLY
  // make sure no one can use the old list of nodes
  mEngine.mContent.getChildByName("sqlite.waffles_nodes").clearChildren();

  // let everyone know we updated the tree of nodes
  mEventClient.notify(CmsDataLoadCompleteEvent());
}

void NWQueryHandler::addReference(
    ds::model::ContentModelRef curParent,
    std::map<int, ds::model::ContentModelRef> &overallMap) {
  for (auto it : curParent.getChildren()) {
    overallMap[it.getId()] = it;
    addReference(it, overallMap);
  }
}

void NWQueryHandler::parseModelProperties(
    ds::model::ContentModelRef &it,
    std::vector<ds::model::ContentModelRef> &allNodes) {
  // add all fields as properties of the node
  auto tags = mEngine.mContent.getChildByName("cms_tags");
  for (auto props : it.getChildren()) {
    auto appKey = props.getPropertyString("app_key");
    auto d_label = it.getPropertyString("name");
    auto d_kind = it.getPropertyString("kind");
    if (d_kind == "template_solution") {
      // DS_LOG_INFO("STREAM");
    }
    auto propName = props.getName();
    if (propName == "waffles_text_fields" || propName == "waffles_checkboxes" ||
        propName == "waffles_color") {
      auto theValue = props.getPropertyString("value");
      auto carriage = theValue.rfind("\r\n");
      if (carriage == theValue.size() - 2) {
        theValue = theValue.substr(0, theValue.size() - 2);
      }
      if (!theValue.empty() && theValue.rfind("\n") == theValue.size() - 1) {
        theValue = theValue.substr(0, theValue.size() - 1);
      }
      it.setProperty(appKey, theValue);
      auto pv = props.getPropertyString("plain_value");

      if (!pv.empty() && theValue.empty()) {
        // DS_LOG_INFO("Text Field:" << appKey << ": plain value is set but
        // value is not!" <<std::endl<<"\t pv:"<<pv<<std::endl<<"\t
        // v:"<<theValue );
      }
      it.setProperty(appKey + "_plain_value", pv);
    } else if (propName == "waffles_media_fields") {
      // DS_LOG_INFO("Processing media for " << d_label);
      auto theMR = processResource(props.getPropertyResource("media_res"));
      auto theTMR =
          processResource(props.getPropertyResource("media_thumb_res"));

      // skip media fields where the resource is empty (deleted or not
      // downloaded yet)
      if (!theMR.empty() && !theTMR.empty()) {
        it.setProperty(appKey + "_media_res",
                       props.getPropertyString("media_res"));
        it.setPropertyResource(appKey + "_media_res", theMR);
        it.setProperty(appKey + "_thumb_res",
                       props.getPropertyString("media_thumb_res"));
        it.setPropertyResource(appKey + "_thumb_res", theTMR);
        if (appKey == "media") {
          it.setPropertyResource(
              "media_res", theMR); // map a primary "media" to be "media_res"
        }
        if (props.getPropertyBool("floatable")) {
          it.setProperty(appKey + "_float_fullscreen",
                         props.getPropertyString("float_fullscreen"));
          it.setProperty(appKey + "_float_autoplay",
                         props.getPropertyString("float_autoplay"));
          it.setProperty(appKey + "_float_loop",
                         props.getPropertyString("float_loop"));
          it.setProperty(appKey + "_float_volume",
                         props.getPropertyString("float_volume"));
          it.setProperty(appKey + "_float_touch",
                         props.getPropertyString("float_touch"));
          it.setProperty(appKey + "_float_page",
                         props.getPropertyString("float_page"));
          it.setProperty(appKey + "_floatable",
                         props.getPropertyString("floatable"));
          it.setProperty(appKey + "_hide_thumbnail",
                         props.getPropertyString("hide_thumbnail"));
        }
      }

    } else if (propName == "waffles_dropdown_fields") {
      if (props.getPropertyBool("allow_multiple")) {
        it.addPropertyToList(appKey,
                             props.getPropertyString("option_save_value"));
      } else {
        it.setProperty(appKey, props.getPropertyString("option_save_value"));
        it.setProperty(appKey + "_dropdown_label",
                       props.getPropertyString("dropdown_label"));
      }

    } else if (propName == "waffles_selections") {
      if (appKey == "briefing_pinboard") {
        auto definitionId = props.getPropertyString("definition_id");
        mEngine.mContent.setProperty("briefing_pinboard_definition_id",
                                     definitionId);
      }

      auto id = props.getPropertyInt("node_id");
      if (std::find_if(allNodes.begin(), allNodes.end(),
                       [id](ds::model::ContentModelRef node) {
                         return node.getId() == id;
                       }) != allNodes.end()) {

        if (props.getPropertyBool("allow_multiple")) {
          it.addPropertyToList(appKey, props.getPropertyString("node_id"));
        } else {
          it.setProperty(appKey, props.getPropertyString("node_id"));
        }
      }
    } else if (propName == "waffles_hotspots") {
      it.setProperty(appKey + "_shape", props.getPropertyString("shape"));
      it.setProperty(appKey + "_pos_x", props.getPropertyString("pos_x"));
      it.setProperty(appKey + "_pos_y", props.getPropertyString("pos_y"));
      it.setProperty(appKey + "_pos_w", props.getPropertyString("pos_w"));
      it.setProperty(appKey + "_pos_h", props.getPropertyString("pos_h"));
    } else if (propName == "waffles_composites") {
      it.setProperty("composite_pos_x",
                     props.getPropertyString(
                         "pos_x")); // todo: handle multiple composites?
      it.setProperty("composite_pos_y", props.getPropertyString("pos_y"));
      it.setProperty("composite_pos_w", props.getPropertyString("pos_w"));
    } else if (propName == "waffles_composite_details") {
      it.setProperty(appKey + "_preview_res",
                     props.getPropertyString("preview_res"));
      it.setPropertyResource(
          appKey + "_preview_res",
          processResource(props.getPropertyResource("preview_res")));
      it.setProperty(appKey + "_preview_thumb_res",
                     props.getPropertyString("preview_thumb_res"));
      it.setPropertyResource(
          appKey + "_preview_thumb_res",
          processResource(props.getPropertyResource("preview_thumb_res")));
    } else if (propName == "waffles_streamconfs") {
      auto theLocation = props.getPropertyString("location");
      if (!theLocation.empty()) {
        auto theResouce = props.getPropertyResource("stream_res");
        theResouce.setFileName(props.getPropertyString("media_title"));
        theResouce.setLocalFilePath(theLocation);
        it.setPropertyResource(props.getPropertyString("resourcesfilename"),
                               theResouce);
      }
    } else if (propName == "waffles_tags") {
      auto tag_class = props.getPropertyString("class");
      auto tag_title = props.getPropertyString("title");
      it.addPropertyToList(props.getPropertyString("taggable_field"),
                           props.getPropertyString("title"));
      it.addPropertyToList("tags_" + tag_class, tag_title);
      auto tag_list = tags.getPropertyList(tag_class);
      bool found = false;
      for (auto tag_item : tag_list) {
        auto id = tag_item.getString();
        if (id == tag_title) {
          found = true;
        }
      }
      if (!found) {
        tags.addPropertyToList(tag_class, tag_title);
      }
    }
  }

  it.clearChildren();
}

ds::Resource NWQueryHandler::processResource(ds::Resource input) {
  auto output = input;
  if (input.getType() == ds::Resource::WEB_TYPE &&
      input.getFileName().find("/youtube/") != std::string::npos) {

    std::string theFilename = input.getFileName();
    auto yt_loc = theFilename.find("/youtube/");
    theFilename = theFilename.substr(yt_loc + 9);
    output.setFileName(theFilename);
    output.setType(ds::Resource::YOUTUBE_TYPE);
  } else if (input.getType() == ds::Resource::VIDEO_STREAM_TYPE) {

    auto allPlatforms = mEngine.mContent.getChildByName("cms_platforms");
    if (allPlatforms.hasChildren()) {
      // the input's filename will be the app key of the platform's stream
      // resource, if it has a stream configuration
      DS_LOG_INFO("STREAM FILENAME:" << input.getFileName());
      auto result =
          allPlatforms.getChild(0).getPropertyResource(input.getFileName());
      return result;
    }

    return ds::Resource();
  }

  return output;
}

} // namespace ds::model
