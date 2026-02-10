import sys
from html.parser import HTMLParser

class MDHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.md = ""
        self.in_pre = False
        self.current_href = ""
        self.in_link = False

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag in ['h1']:
            self.md += "\n# "
        elif tag in ['h2']:
            self.md += "\n## "
        elif tag in ['h3']:
            self.md += "\n### "
        elif tag in ['h4']:
            self.md += "\n#### "
        elif tag in ['p']:
            self.md += "\n\n"
        elif tag in ['br']:
            self.md += "  \n"
        elif tag in ['b', 'strong']:
            self.md += "**"
        elif tag in ['i', 'em']:
            self.md += "*"
        elif tag in ['code', 'tt']:
            self.md += "`"
        elif tag in ['pre']:
            self.md += "\n```\n"
            self.in_pre = True
        elif tag in ['li']:
            self.md += "\n- "
        elif tag in ['a']:
            self.current_href = attrs.get('href', '')
            self.md += "["
            self.in_link = True

    def handle_endtag(self, tag):
        if tag in ['h1', 'h2', 'h3', 'h4']:
            self.md += "\n"
        elif tag in ['p']:
            self.md += "\n"
        elif tag in ['b', 'strong']:
            self.md += "**"
        elif tag in ['i', 'em']:
            self.md += "*"
        elif tag in ['code', 'tt']:
            self.md += "`"
        elif tag in ['pre']:
            self.md += "\n```\n"
            self.in_pre = False
        elif tag in ['a']:
            self.md += f"]({self.current_href})"
            self.in_link = False

    def handle_data(self, data):
        if self.in_pre:
            self.md += data
        else:
            # Collapse multiple spaces but preserve single spaces if needed,
            # but mainly remove newlines which are just formatting in HTML source
            clean_data = ' '.join(data.split())
            if clean_data:
                self.md += clean_data + " "

parser = MDHTMLParser()

try:
    with open("docs/commandref.html", "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    
    parser.feed(content)
    
    # Simple post-processing
    final_md = parser.md
    
    with open("docs/commandref.md", "w", encoding="utf-8") as f:
        f.write(final_md)
        
    print("Conversion complete.")
except Exception as e:
    print(f"Error: {e}")
