#!/usr/bin/env python3
import hashlib
import pathlib
import re
import sys

project = sys.argv[1] if len(sys.argv) > 1 else None
roots = [pathlib.Path('src')]
roots.append(pathlib.Path('include') / project if project else pathlib.Path('include'))

func_re = re.compile(r'''(?ms)^\s*(?:static\s+)?(?:inline\s+)?[A-Za-z_][\w\s\*]+\s+([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{''')

def strip_noise(body: str) -> str:
    body = re.sub(r'/\*.*?\*/', '', body, flags=re.S)
    body = re.sub(r'//.*', '', body)
    body = re.sub(r'"(?:\\.|[^"\\])*"', '"STR"', body)
    body = re.sub(r"'(?:\\.|[^'\\])*'", "'CHR'", body)
    body = re.sub(r'\b0x[0-9a-fA-F]+\b|\b\d+\b', 'NUM', body)
    body = re.sub(r'\s+', ' ', body).strip()
    return body

def find_body(text, open_brace):
    depth = 0
    for i in range(open_brace, len(text)):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return text[open_brace:i+1]
    return None

groups = {}
for root in roots:
    if not root.exists():
        continue
    for path in root.rglob('*'):
        if path.suffix not in {'.c', '.cc', '.cpp', '.h', '.hpp'}:
            continue
        text = path.read_text(errors='ignore')
        for m in func_re.finditer(text):
            body = find_body(text, m.end() - 1)
            if not body:
                continue
            norm = strip_noise(body)
            if len(norm) < 120:
                continue
            h = hashlib.sha256(norm.encode()).hexdigest()[:16]
            groups.setdefault(h, []).append((str(path), m.group(1), len(norm)))

bad = 0
for h, items in sorted(groups.items(), key=lambda kv: -len(kv[1])):
    names = {name for _, name, _ in items}
    if len(items) >= 3 and len(names) >= 3:
        bad += 1
        print(f'DUPLICATE_BODY_GROUP {h} count={len(items)}')
        for path, name, size in items[:20]:
            print(f'  {path}:{name} norm_size={size}')

if bad:
    print('FAIL: repeated function bodies detected; rewrite as real abstractions or remove padding', file=sys.stderr)
    sys.exit(1)
print('OK: no large repeated function-body groups detected')
