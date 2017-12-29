#include "stdafx.h"

#include "markdown_to_pango.h"
#include "string_util.h"

#include "ds/util/sundown/markdown.h"

namespace ds {
namespace ui {

static void rndr_normal_text(struct buf *ob, const struct buf *text, void *opaque){
	bufput(ob, text->data, text->size);
}

static void rndr_paragraph(struct buf *ob, const struct buf *text, void *opaque) {
	bufput(ob, text->data, text->size);
	bufputs(ob, "\n\n");
}
static int rndr_strikethrough(struct buf *ob, const struct buf *text, void *opaque) {
	if(!text || !text->size) return 0;
	bufputs(ob, "<span strikethrough='true'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>");
	return 1;
}
static int rndr_superscript(struct buf *ob, const struct buf *text, void *opaque) {
	if(!text || !text->size) return 0;
	bufputs(ob, "<sup>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</sup>");
	return 1;
}
static int rndr_double_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
	if(!text || !text->size) return 0;
	bufputs(ob, "<span weight='bold'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>");
	return 1;
}
static int rndr_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
	if(!text || !text->size) return 0;
	bufputs(ob, "<span style='oblique'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>");
	return 1;
}

static int rndr_triple_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
	if(!text || !text->size) return 0;
	bufputs(ob, "<span weight='bold' style='oblique'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>");
	return 1;
}

static void rndr_header(struct buf *ob, const struct buf *text, int level, void *opaque) {
	// if we want more space in front of headers
	if(ob->size) {
	//	bufputc(ob, '\n');
	}

	if(level == 1) bufputs(ob, "<span weight='heavy' size='xx-large'>");
	else if(level == 2) bufputs(ob, "<span weight='heavy' size='x-large'>");
	else bufputs(ob, "<span weight='heavy' size='large'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>\n\n");
}

static void rndr_list(struct buf *ob, const struct buf *text, int flags, void *opaque){
	if(flags & MKD_LIST_ORDERED) {
		bufputs(ob, "<ol>\n");
	} else {
		bufputs(ob, "<ul>\n");
	}
	if(text) bufput(ob, text->data, text->size);

	if(flags & MKD_LIST_ORDERED) {
		bufputs(ob, "\n</ol>\n");
	} else {
		bufputs(ob, "\n</ul>\n");
	}
	bufputc(ob, '\n');	
}

static void rndr_listitem(struct buf *ob, const struct buf *text, int flags, void *opaque){
	bufputs(ob, "&bull;");
	if(text) {
		size_t size = text->size;
		while(size && text->data[size - 1] == '\n')
			size--;

		bufput(ob, text->data, size);
	}
	bufputs(ob, "\n");
}

static int rndr_linebreak(struct buf *ob, void *opaque){
	bufputs(ob, "\n\n");
	return 1;
}

static void rndr_blockquote(struct buf *ob, const struct buf *text, void *opaque){
	BUFPUTSL(ob, "<blockquote><span font='Courier New'>");
	if(text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</span></blockquote>\n");
}

static void rndr_blockcode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque) {
	BUFPUTSL(ob, "<blockquote><span font='Consolas' >");
	if(text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</span></blockquote>\n");
}

static int rndr_codespan(struct buf *ob, const struct buf *text, void *opaque) {
	BUFPUTSL(ob, "<span font='Consolas'>");
	if(text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</span>");
	return 1;
}

static void rndr_table(struct buf *ob, const struct buf *header, const struct buf *body, void *opaque){
	BUFPUTSL(ob, "<span font='Consolas'>");
	if(header)
		bufput(ob, header->data, header->size);
	if(body)
		bufput(ob, body->data, body->size);
	BUFPUTSL(ob, "</span>");
	bufputc(ob, '\n');
}

static void rndr_tablerow(struct buf *ob, const struct buf *text, void *opaque){
	//BUFPUTSL(ob, "<tr>\n");
	if(text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "\n");
}

static void rndr_tablecell(struct buf *ob, const struct buf *text, int flags, void *opaque){
	if(text) 
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, " | ");
}

std::wstring markdown_to_pango(const std::wstring& inputMarkdown) {
	return ds::wstr_from_utf8(markdown_to_pango(ds::utf8_from_wstr(inputMarkdown)));
}

std::string markdown_to_pango(const std::string& source) {
	
	// see the html directory for usage
	// https://github.com/apiaryio/sundown/


	static const struct sd_callbacks cb_default = {

		/// NULL skips these ones (note: tables aren't parsed)
		rndr_blockcode, // rndr_blockcode,
		rndr_blockquote, // rndr_blockquote,
		rndr_normal_text, // rndr_raw_block,
		rndr_header, // rndr_header,
		NULL, // rndr_hrule,
		rndr_list, // rndr_list,
		rndr_listitem, // rndr_listitem,
		rndr_paragraph, // rndr_paragraph,
		rndr_table, // rndr_table,
		rndr_tablerow, // rndr_tablerow,
		rndr_tablecell, // rndr_tablecell,

		/// NULL or returning 0 adds the original text verbatim
		NULL, // rndr_autolink,
		rndr_codespan, // rndr_codespan,
		rndr_double_emphasis, // rndr_double_emphasis,
		rndr_emphasis, // rndr_emphasis,
		NULL, // rndr_image,
		rndr_linebreak, // rndr_linebreak,
		NULL, // rndr_link,
		NULL, // rndr_raw_html,
		rndr_triple_emphasis, // rndr_triple_emphasis,
		rndr_strikethrough, // rndr_strikethrough,
		rndr_superscript, // rndr_superscript,


		/// These all return the original if NULL
		NULL, // entity - copied directly
		rndr_normal_text,

		NULL, // header - copied directly
		NULL, // footer - copied directly
	};

	struct sd_callbacks callbacks;
	struct sd_markdown *markdown;
	struct buf *ob;
	callbacks = cb_default;

	const int parserExtensions = MKDEXT_FENCED_CODE | MKDEXT_NO_INTRA_EMPHASIS | MKDEXT_LAX_SPACING | MKDEXT_SUPERSCRIPT | MKDEXT_STRIKETHROUGH  /*| MKDEXT_TABLES */;
	markdown = sd_markdown_new(parserExtensions, 16, &callbacks, nullptr);


	std::string theInput = source;
	
	// Remove some "offensive" characters
	// These will break pango's markup later on
	// These also break blockquotes, but we re-add those later
	ds::replace(theInput, "&", "&amp;");
	ds::replace(theInput, "<", "&lt;");
	ds::replace(theInput, ">", "&gt;");

	ob = bufnew(64);
	sd_markdown_render(ob, reinterpret_cast<const uint8_t*>(theInput.c_str()), theInput.length(), markdown);
	sd_markdown_free(markdown);

	if(!ob || !ob->data || !ob->size) return "";

	std::string outputString = std::string(reinterpret_cast<char*>(ob->data), ob->size);

	bufrelease(ob);

	std::string outputty;


	// We've got to do some post-processing for lists and other element types, due to the way the parser handles lists
	auto lines = ds::split(outputString, "\n", false);
	int indent = 0;

	struct ListType {
		ListType(const int typey) :listType(typey), listCount(1) {}

		int listType;
		int listCount;
	};

	// 0 = unordered list
	// 1 = ordered list
	std::vector<ListType> listTypes;

	for (auto it : lines){

		std::string thisLine = it;

		if(thisLine.find("<ol>") != std::string::npos) {
			listTypes.push_back(ListType(1));
			ds::replace(thisLine, "<ol>", "");
			indent++;
			continue;
		} else if(thisLine.find("<ul>") != std::string::npos) {
			listTypes.push_back(ListType(0));
			ds::replace(thisLine, "<ul>", "");
			indent++;
			continue;
		} else if(thisLine.find("</ol>") != std::string::npos) {
			indent--;
			if(!listTypes.empty()) listTypes.pop_back();
			continue;
		} else if(thisLine.find("</ul>") != std::string::npos) {
			indent--;
			if(!listTypes.empty()) listTypes.pop_back();
			continue;
		}

		if(thisLine.find("<blockquote>") != std::string::npos) {
			ds::replace(thisLine, "<blockquote>", "");
			indent++;
			if(indent == 1) indent++;
		} else if(thisLine.find("</blockquote>") != std::string::npos) {
			ds::replace(thisLine, "</blockquote>", "");
			indent--;
			if(indent == 1) indent--;
		}

		// in case the parser effed up
		if(indent < 0) indent = 0;

		/// this fixes a bug where there could be an extra line when dropping down a level in indentation
		if(!listTypes.empty() && thisLine.empty()) continue;

		// blockquote parsing got smushed by our > find/replace earlier
		if(listTypes.empty() &&  thisLine.find("&gt;") == 0) {
			thisLine.replace(0, 4, "    <span font='Palatino Italic'>");
			thisLine.append("</span>");
		}

		for(int i = 1; i < indent; i++) {
			outputty.append("	");
		}

		// to properly encode bullets, add it here using a wstr
		if(thisLine.find("&bull;") != std::string::npos) {
			if(listTypes.empty() || listTypes.back().listType == 0) {
				ds::replace(thisLine, "&bull;", ds::utf8_from_wstr(L"\u2022 "));
			} else if(listTypes.back().listType == 1) {
				ds::replace(thisLine, "&bull;", std::to_string(listTypes.back().listCount) + ". ");
				listTypes.back().listCount++;
			}  
		}

		outputty.append(thisLine);
		outputty.append("\n");
	}

	return outputty;
}

}
}