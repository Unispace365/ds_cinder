#pragma once
#ifndef DS_UTIL_MARKDOWN_TO_PANGO
#define DS_UTIL_MARKDOWN_TO_PANGO

#include <string>

namespace ds { namespace ui {

	/// It'd be cool if this supported custom callbacks for certain elements for further styling
	///	For instance, you could specify your own font or background color for code blocks or headers, etc.
	///
	///	I agree, so here it is. Note: to prevent unnecessary memory allocations, we're not setting the member variables
	/// to default values on purpose.
	struct MarkdownOptions {
		struct Option {
			std::string open;
			std::string close;
			Option() = default;
			Option(std::string_view o, std::string_view c)
			  : open(o)
			  , close(c) {}
		};

		Option paragraph;	  //
		Option header1;		  //
		Option header2;		  //
		Option header3;		  //
		Option header4;		  //
		Option header5;		  //
		Option header6;		  //
		Option superscript;	  //
		Option emphasis1;	  //
		Option emphasis2;	  // Double emphasis.
		Option emphasis3;	  // Triple emphasis.
		Option strikethrough; // Strike through.
		Option ordered;		  // Ordered list.
		Option unordered;	  // unordered list.
		Option blockquote;	  // blockquote.
		Option blockcode;	  // Code block.
		Option codespan;	  // Code span.
		Option table;		  // Table.

		static MarkdownOptions create() {
			return {
				{"", "\n\n"},														 // paragraph
				{"<span weight='bold' size='xx-large'>", "</span>"},				 // header1
				{"<span weight='bold' size='x-large'>", "</span>"},					 // header2
				{"<span weight='bold' size='large'>", "</span>"},					 // header3
				{"<span weight='bold' size='medium'>", "</span>"},					 // header4
				{"<span weight='bold' size='small'>", "</span>"},					 // header5
				{"<span weight='bold' size='x-small'>", "</span>"},					 // header6
				{"<sup>", "</sup>"},												 // superscript
				{"<span style='oblique'>", "</span>"},								 // emphasis1
				{"<span weight='bold'>", "</span>"},								 // emphasis2
				{"<span weight='bold' style='oblique'>", "</span>"},				 // emphasis3
				{"<span strikethrough='true'>", "</span>"},							 // strikethrough
				{"<ol>\n", "\n</ol>\n\n"},											 // ordered
				{"<ul>\n", "\n</ul>\n\n"},											 // unordered
				{"<blockquote><span font='Courier New'>", "</span></blockquote>\n"}, // blockquote
				{"<blockquote><span font='Consolas' >", "</span></blockquote>\n"},	 // blockcode
				{"<span font='Consolas'>", "</span>"},								 // codespan
				{"<span font='Consolas'>", "</span>\n"}								 // table
			};
		}

		static const MarkdownOptions& defaults() { return sDefaultOptions; }

		static void setDefaults(const MarkdownOptions& options) { sDefaultOptions = options; }

	  private:
		static MarkdownOptions sDefaultOptions;
	};


	std::wstring markdown_to_pango(const std::wstring&	  inputMarkdown,
								   const MarkdownOptions& options = MarkdownOptions::defaults());
	std::string	 markdown_to_pango(const std::string&	  inputMarkdown,
								   const MarkdownOptions& options = MarkdownOptions::defaults());
}} // namespace ds::ui

#endif // DS_UTIL_STRINGUTIL_H_
