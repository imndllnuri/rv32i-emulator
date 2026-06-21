import re

from PyQt5.QtCore import QRegExp, Qt
from PyQt5.QtGui import QColor, QFont, QSyntaxHighlighter, QTextCharFormat

from disassembler import ABI_NAMES, ALL_MNEMONICS

_DIRECTIVES = [
    ".text", ".data", ".bss", ".global", ".globl", ".align", ".word",
    ".half", ".byte", ".ascii", ".asciz", ".string", ".equ", ".section",
    ".extern", ".size", ".type",
]


def _fmt(color, bold=False, italic=False):
    f = QTextCharFormat()
    f.setForeground(QColor(color))
    if bold:
        f.setFontWeight(QFont.Bold)
    f.setFontItalic(italic)
    return f


class AsmHighlighter(QSyntaxHighlighter):
    """Lightweight syntax highlighter for RISC-V assembly source."""

    def __init__(self, document):
        super().__init__(document)

        self._mnemonic_fmt = _fmt("#569CD6", bold=True)
        self._register_fmt = _fmt("#9CDCFE")
        self._directive_fmt = _fmt("#C586C0")
        self._comment_fmt = _fmt("#6A9955", italic=True)
        self._number_fmt = _fmt("#B5CEA8")
        self._label_fmt = _fmt("#DCDCAA", bold=True)
        self._string_fmt = _fmt("#CE9178")

        self._rules = []

        mnem_pattern = r"\b(" + "|".join(re.escape(m) for m in ALL_MNEMONICS) + r")\b"
        mnem_regexp = QRegExp(mnem_pattern)
        mnem_regexp.setCaseSensitivity(Qt.CaseInsensitive)
        self._rules.append((mnem_regexp, self._mnemonic_fmt))

        reg_pattern = r"\b(" + "|".join(ABI_NAMES) + r"|x[0-9]|x[12][0-9]|x3[01])\b"
        self._rules.append((QRegExp(reg_pattern), self._register_fmt))

        dir_pattern = r"(" + "|".join(re.escape(d) for d in _DIRECTIVES) + r")\b"
        self._rules.append((QRegExp(dir_pattern), self._directive_fmt))

        self._rules.append((QRegExp(r"\b0[xX][0-9a-fA-F]+\b"), self._number_fmt))
        self._rules.append((QRegExp(r"\b-?[0-9]+\b"), self._number_fmt))

        self._rules.append((QRegExp(r"^\s*[A-Za-z_.][A-Za-z0-9_.]*:"), self._label_fmt))

        self._rules.append((QRegExp(r'"[^"]*"'), self._string_fmt))

        # Comment patterns are applied last so they win over everything else.
        self._comment_rule = QRegExp(r"(#|//).*")

    def highlightBlock(self, text):
        for pattern, fmt in self._rules:
            index = pattern.indexIn(text)
            while index >= 0:
                length = pattern.matchedLength()
                self.setFormat(index, length, fmt)
                index = pattern.indexIn(text, index + length)

        index = self._comment_rule.indexIn(text)
        if index >= 0:
            self.setFormat(index, len(text) - index, self._comment_fmt)
