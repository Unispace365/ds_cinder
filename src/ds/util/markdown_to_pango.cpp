#include "stdafx.h"

#include "markdown_to_pango.h"
#include "string_util.h"

#include "ds/util/sundown/markdown.h"

namespace ds {
namespace ui {



std::wstring markdown_to_pango(const std::wstring& inputMarkdown) {

	return inputMarkdown;
}
static void rndr_normal_text(struct buf *ob, const struct buf *text, void *opaque){
	bufput(ob, text->data, text->size);
}

static void rndr_paragraph(struct buf *ob, const struct buf *text, void *opaque) {
	bufput(ob, text->data, text->size);
	bufputs(ob, "\n");
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
	if(ob->size) {
		bufputc(ob, '\n');
	}

	if(level == 1) bufputs(ob, "<span weight='heavy' size='xx-large'>");
	else if(level == 2) bufputs(ob, "<span weight='heavy' size='x-large'>");
	else bufputs(ob, "<span weight='heavy' size='large'>");
	bufput(ob, text->data, text->size);
	bufputs(ob, "</span>\n");
}

static void rndr_list(struct buf *ob, const struct buf *text, int flags, void *opaque){
	if(ob->size) bufputc(ob, '\n');
	//bufput(ob, flags & MKD_LIST_ORDERED ? "<ol>\n" : "<ul>\n", 5);
	if(text) bufput(ob, text->data, text->size);
	if(ob->size) bufputc(ob, '\n');
	//bufput(ob, flags & MKD_LIST_ORDERED ? "</ol>\n" : "</ul>\n", 6);
	
}

static void rndr_listitem(struct buf *ob, const struct buf *text, int flags, void *opaque){
	bufputs(ob, "&bull; ");
	if(text) {
		size_t size = text->size;
		while(size && text->data[size - 1] == '\n')
			size--;

		bufput(ob, text->data, size);
	}
	bufputs(ob, "\n");
}


std::string markdown_to_pango(const std::string& inputMarkdown) {
//	auto lines = ds::split(inputMarkdown, L"\n");

	struct sd_callbacks callbacks;
	//struct html_renderopt options;
	struct sd_markdown *markdown;
	struct buf *ib, *ob;
	int ret;


	static const struct sd_callbacks cb_default = {
		NULL, // rndr_blockcode,
		NULL, // rndr_blockquote,
		NULL, // rndr_raw_block,
		rndr_header, // rndr_header,
		NULL, // rndr_hrule,
		rndr_list, // rndr_list,
		rndr_listitem, // rndr_listitem,
		rndr_paragraph, // rndr_paragraph,
		NULL, // rndr_table,
		NULL, // rndr_tablerow,
		NULL, // rndr_tablecell,

		NULL, // rndr_autolink,
		NULL, // rndr_codespan,
		rndr_double_emphasis, // rndr_double_emphasis,
		rndr_emphasis, // rndr_emphasis,
		NULL, // rndr_image,
		NULL, // rndr_linebreak,
		NULL, // rndr_link,
		NULL, // rndr_raw_html,
		rndr_triple_emphasis, // rndr_triple_emphasis,
		NULL, // rndr_strikethrough,
		NULL, // rndr_superscript,

		NULL,
		rndr_normal_text,

		NULL,
		NULL,
	};

	callbacks = cb_default;

	FILE *in = stdin;

	in = fopen(inputMarkdown.c_str(), "r");
	if(!in) {
		//fprintf(stderr, "Unable to open input file \"%s\": %s\n", argv[1], strerror(errno));
		return "WHOOPS";
	}

	/* reading everything */
	ib = bufnew(1024);
	bufgrow(ib, 1024);
	while((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, in)) > 0) {
		ib->size += ret;
		bufgrow(ib, ib->size + 1024);
	}

	if(in != stdin)
	fclose(in);


	markdown = sd_markdown_new(0, 16, &callbacks, nullptr);

	ob = bufnew(64);

	sd_markdown_render(ob, ib->data, ib->size, markdown);

	sd_markdown_free(markdown);

	//fwrite(ob->data, 1, ob->size, stdout);

	std::stringstream ss;
	for(int i = 0; i < ob->size; i++) {
		ss << ob->data[i];
	}

	//std::cout << ss.str() << std::endl;

	bufrelease(ib);
	bufrelease(ob);

	return ss.str();
}

}
}