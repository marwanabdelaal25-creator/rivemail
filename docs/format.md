# Message handling notes

rivemail separates message handling into header parsing, MIME field interpretation, optional normalization, validation, and rendering. A caller can parse a message once and then inspect headers without forcing body decoding. Multipart details are represented conservatively so applications can decide how much structure they want to expose.
