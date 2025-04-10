#include "stdafx.h"

#include "markdown_to_pango.h"
#include "string_util.h"

#include "ds/util/sundown/markdown.h"
#include <cctype>
#include <locale>

namespace ds { namespace ui {

	MarkdownOptions MarkdownOptions::sDefaultOptions = MarkdownOptions::create();

	static void rndr_normal_text(buf* ob, const buf* text, void* options) {
		bufput(ob, text->data, text->size);
	}

	static void rndr_paragraph(buf* ob, const buf* text, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->paragraph.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->paragraph.close.c_str());
	}
	static int rndr_strikethrough(buf* ob, const buf* text, void* options) {
		if (!text || !text->size) return 0;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->strikethrough.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->strikethrough.close.c_str());
		return 1;
	}
	static int rndr_superscript(buf* ob, const buf* text, void* options) {
		if (!text || !text->size) return 0;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->superscript.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->superscript.close.c_str());
		return 1;
	}
	static int rndr_double_emphasis(buf* ob, const buf* text, void* options) {
		if (!text || !text->size) return 0;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->emphasis2.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->emphasis2.close.c_str());
		return 1;
	}
	static int rndr_emphasis(buf* ob, const buf* text, void* options) {
		if (!text || !text->size) return 0;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->emphasis1.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->emphasis1.close.c_str());
		return 1;
	}

	static int rndr_triple_emphasis(buf* ob, const buf* text, void* options) {
		if (!text || !text->size) return 0;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->emphasis3.open.c_str());
		bufput(ob, text->data, text->size);
		bufputs(ob, opts->emphasis3.close.c_str());
		return 1;
	}

	static void rndr_header(buf* ob, const buf* text, int level, void* options) {
		if (!text || !text->size) return;
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		switch (level) {
		case 1:
			bufputs(ob, opts->header1.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header1.close.c_str());
			break;
		case 2:
			bufputs(ob, opts->header2.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header2.close.c_str());
			break;
		case 3:
			bufputs(ob, opts->header3.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header3.close.c_str());
			break;
		case 4:
			bufputs(ob, opts->header4.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header4.close.c_str());
			break;
		case 5:
			bufputs(ob, opts->header5.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header5.close.c_str());
			break;
		default:
			bufputs(ob, opts->header6.open.c_str());
			bufput(ob, text->data, text->size);
			bufputs(ob, opts->header6.close.c_str());
			break;
		}
	}

	static void rndr_list(buf* ob, const buf* text, int flags, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		if (flags & MKD_LIST_ORDERED) {
			bufputs(ob, opts->ordered.open.c_str());
			if (text) bufput(ob, text->data, text->size);
			bufputs(ob, opts->ordered.close.c_str());
		} else {
			bufputs(ob, opts->unordered.open.c_str());
			if (text) bufput(ob, text->data, text->size);
			bufputs(ob, opts->unordered.close.c_str());
		}
	}

	static void rndr_listitem(buf* ob, const buf* text, int flags, void* options) {
		BUFPUTSL(ob, "&bull;");
		if (text) {
			size_t size = text->size;
			while (size && text->data[size - 1] == '\n')
				size--;

			bufput(ob, text->data, size);
		}
		BUFPUTSL(ob, "\n");
	}

	static int rndr_linebreak(buf* ob, void* options) {
		BUFPUTSL(ob, "\n\n");
		return 1;
	}

	static void rndr_blockquote(buf* ob, const buf* text, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->blockquote.open.c_str());
		if (text) bufput(ob, text->data, text->size);
		bufputs(ob, opts->blockquote.close.c_str());
	}

	static void rndr_blockcode(buf* ob, const buf* text, const buf* lang, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->blockcode.open.c_str());
		if (text) bufput(ob, text->data, text->size);
		bufputs(ob, opts->blockcode.close.c_str());
	}

	static int rndr_codespan(buf* ob, const buf* text, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->codespan.open.c_str());
		if (text) bufput(ob, text->data, text->size);
		bufputs(ob, opts->codespan.close.c_str());
		return 1;
	}

	static void rndr_table(buf* ob, const buf* header, const buf* body, void* options) {
		const MarkdownOptions* opts = static_cast<const MarkdownOptions*>(options);
		bufputs(ob, opts->table.open.c_str());
		if (header) bufput(ob, header->data, header->size);
		if (body) bufput(ob, body->data, body->size);
		bufputs(ob, opts->table.close.c_str());
		bufputc(ob, '\n');
	}

	static void rndr_tablerow(buf* ob, const buf* text, void* options) {
		// BUFPUTSL(ob, "<tr>\n");
		if (text) bufput(ob, text->data, text->size);
		BUFPUTSL(ob, "\n");
	}

	static void rndr_tablecell(buf* ob, const buf* text, int flags, void* options) {
		if (text) bufput(ob, text->data, text->size);
		BUFPUTSL(ob, " | ");
	}

	static int rndr_raw_html(buf* ob, const buf* text, void* options) {
		/* if (text) bufput(ob, text->data, text->size);
		BUFPUTSL(ob, " | "); */
		return 0;
	}

	std::wstring markdown_to_pango(const std::wstring& inputMarkdown, const MarkdownOptions& options) {
		return ds::wstr_from_utf8(markdown_to_pango(ds::utf8_from_wstr(inputMarkdown), options));
	}

	std::string markdown_to_pango(const std::string& inputMarkdown, const MarkdownOptions& options) {

		// see the html directory for usage
		// https://github.com/apiaryio/sundown/


		static const sd_callbacks cb_default = {

			/// NULL skips these ones (note: tables aren't parsed)
			rndr_blockcode,	  // rndr_blockcode,
			rndr_blockquote,  // rndr_blockquote,
			rndr_normal_text, // rndr_raw_block,
			rndr_header,	  // rndr_header,
			NULL,			  // rndr_hrule,
			rndr_list,		  // rndr_list,
			rndr_listitem,	  // rndr_listitem,
			rndr_paragraph,	  // rndr_paragraph,
			rndr_table,		  // rndr_table,
			rndr_tablerow,	  // rndr_tablerow,
			rndr_tablecell,	  // rndr_tablecell,

			/// NULL or returning 0 adds the original text verbatim
			NULL,				  // rndr_autolink,
			rndr_codespan,		  // rndr_codespan,
			rndr_double_emphasis, // rndr_double_emphasis,
			rndr_emphasis,		  // rndr_emphasis,
			NULL,				  // rndr_image,
			rndr_linebreak,		  // rndr_linebreak,
			NULL,				  // rndr_link,
			rndr_raw_html,		  // rndr_raw_html,
			rndr_triple_emphasis, // rndr_triple_emphasis,
			rndr_strikethrough,	  // rndr_strikethrough,
			rndr_superscript,	  // rndr_superscript,


			/// These all return the original if NULL
			NULL, // entity - copied directly
			rndr_normal_text,

			NULL, // header - copied directly
			NULL, // footer - copied directly
		};

		sd_callbacks callbacks;
		sd_markdown* markdown;
		buf*		 ob;
		callbacks = cb_default;

		constexpr int parserExtensions =
			MKDEXT_FENCED_CODE | MKDEXT_LAX_SPACING | MKDEXT_SUPERSCRIPT | MKDEXT_STRIKETHROUGH /*| MKDEXT_TABLES */;
		markdown = sd_markdown_new(parserExtensions, 16, &callbacks, const_cast<MarkdownOptions*>(&options));

		ob = bufnew(64);
		sd_markdown_render(ob, reinterpret_cast<const uint8_t*>(inputMarkdown.c_str()), inputMarkdown.length(),
						   markdown);
		sd_markdown_free(markdown);

		if (!ob || !ob->data || !ob->size) return {};

		std::string outputString = std::string(reinterpret_cast<char*>(ob->data), ob->size);

		bufrelease(ob);

		// TODO: figure out if we can prevent all the additional parsing below by simply updating the MarkdownOptions.

		std::string outputty;


		// We've got to do some post-processing for lists and other element types, due to the way the parser handles
		// lists
		auto lines	= ds::split(outputString, "\n", false);
		int	 indent = 0;

		struct ListType {
			ListType(const int typey)
			  : listType(typey) {}

			int listType;
			int listCount{1};
		};

		// 0 = unordered list
		// 1 = ordered list
		std::vector<ListType> listTypes;

		for (std::string thisLine : lines) {
			if (thisLine.find("<ol>") != std::string::npos) {
				listTypes.emplace_back(1);
				ds::replace(thisLine, "<ol>", "");
				indent++;
				continue;
			} else if (thisLine.find("<ul>") != std::string::npos) {
				listTypes.emplace_back(0);
				ds::replace(thisLine, "<ul>", "");
				indent++;
				continue;
			} else if (thisLine.find("</ol>") != std::string::npos) {
				indent--;
				if (!listTypes.empty()) listTypes.pop_back();
				continue;
			} else if (thisLine.find("</ul>") != std::string::npos) {
				indent--;
				if (!listTypes.empty()) listTypes.pop_back();
				continue;
			}

			if (thisLine.find("<blockquote>") != std::string::npos) {
				ds::replace(thisLine, "<blockquote>", "");
				indent++;
				if (indent == 1) indent++;
			} else if (thisLine.find("</blockquote>") != std::string::npos) {
				ds::replace(thisLine, "</blockquote>", "");
				indent--;
				if (indent == 1) indent--;
			}

			// in case the parser effed up
			if (indent < 0) indent = 0;

			/// this fixes a bug where there could be an extra line when dropping down a level in indentation
			if (!listTypes.empty() && thisLine.empty()) continue;

			// blockquote parsing got smushed by our > find/replace earlier
			if (listTypes.empty() && thisLine.find("&gt;") == 0) {
				thisLine.replace(0, 4, "    <span font='Palatino Italic'>");
				thisLine.append("</span>");
			}

			for (int i = 1; i < indent; i++) {
				outputty.append("	");
			}

			// to properly encode bullets, add it here using a wstr
			if (thisLine.find("&bull;") != std::string::npos) {
				if (listTypes.empty() || listTypes.back().listType == 0) {
					ds::replace(thisLine, "&bull;", ds::utf8_from_wstr(L"\u2022 "));
				} else if (listTypes.back().listType == 1) {
					ds::replace(thisLine, "&bull;", std::to_string(listTypes.back().listCount) + ". ");
					listTypes.back().listCount++;
				}
			}

			outputty.append(thisLine);
			outputty.append("\n");
		}

		while (!outputty.empty() && std::isspace(outputty.back(), std::locale("en_US.UTF8"))) {
			outputty.pop_back();
		}

		return outputty;
	}

}} // namespace ds::ui
