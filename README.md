# rivemail

rivemail is a small C library and command-line tool for reading, validating, normalizing, and rendering Internet mail messages. It focuses on RFC5322-style headers, common MIME content-type handling, transfer decoding helpers, mailbox address cleanup, and mbox file inspection.

Maintainer: Marwan Helmy  
GitHub: https://github.com/marwanabdelaal25-creator  
Email: marwanabdelaal25@gmail.com

## Build

```sh
make test
make tool
```

The default test build enables AddressSanitizer and UndefinedBehaviorSanitizer when the compiler supports them.

## Command line

```sh
./rivemail headers examples/simple.eml
./rivemail validate examples/simple.eml
./rivemail render examples/simple.eml
./rivemail mbox examples/sample.mbox
```

## Library use

```c
rv_message msg;
rv_string out;
rv_string_init(&out);
if (rv_message_parse(bytes, length, NULL, &msg) == RV_OK) {
    rv_message_render(&msg, &out);
    rv_string_free(&out);
    rv_message_free(&msg);
}
```

## Format notes

The parser accepts CRLF or LF line endings, unfolds continuation lines, records raw headers when requested, and exposes parsed `Content-Type` media type, subtype, and parameters. The renderer writes canonical CRLF line endings and folds long ordinary fields.

## Error handling

APIs return `rv_status`. Objects initialized or filled by the library should be released with the matching free function. Validation warnings are returned separately so callers can decide whether to accept messages with obsolete or incomplete fields.

## Testing

The project includes unit tests and fuzz tests that exercise parser, validation, and renderer paths as part of normal engineering maintenance.

## Limitations

rivemail is not a full mail transfer agent. It does not perform DNS checks, DKIM cryptographic verification, spam filtering, or character-set conversion beyond simple encoded-word and transfer-decoding helpers.

## Versioning

The public C API uses the `rv_` prefix. Backward-compatible additions may be made in minor releases; ABI-breaking changes should be reserved for major releases.
