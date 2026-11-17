"""Validate the published llms.txt and its discovery link in every built page."""
from html.parser import HTMLParser
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / 'website' / 'build'
BASE_URL = os.environ.get('DOCS_BASE_URL', '/BASICGOL/')


class DescribedByParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.found = False

    def handle_starttag(self, tag, attrs):
        attributes = dict(attrs)
        if (tag == 'link' and attributes.get('rel') == 'describedby'
                and attributes.get('href') == BASE_URL + 'llms.txt'):
            self.found = True


def main():
    assert (BUILD / 'llms.txt').read_bytes() == (ROOT / 'llms.txt').read_bytes(), 'llms.txt differs from source'
    pages = list(BUILD.rglob('*.html'))
    assert pages, 'No HTML pages found in build'
    for page in pages:
        parser = DescribedByParser()
        parser.feed(page.read_text(encoding='utf-8'))
        assert parser.found, f'Missing describedby link: {page}'
    print(f'llms.txt and discovery links verified in {len(pages)} pages')


if __name__ == '__main__':
    main()
