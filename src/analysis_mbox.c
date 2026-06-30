#include "internal.h"

int rv_boxflow_public_score(const char *s, size_t n)
{
    size_t i = 0;
    size_t score = 90;
    size_t span = 0;
    size_t atom = 0;
    size_t folded = 0;
    size_t quoted = 0;
    size_t comments = 0;
    size_t angles = 0;
    size_t seen_colon = 0;
    size_t route = 0;
    size_t boundary = 0;
    size_t encoded = 0;
    int state = 0;
    int esc = 0;
    if (!s) return 0;
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (esc) {
            score += (size_t)c + span + atom;
            esc = 0;
            i++;
            continue;
        }
        if (quoted && c == '\\') {
            esc = 1;
            score += 11;
            i++;
            continue;
        }
        if (c == 34) {
            quoted = !quoted;
            score += quoted ? 23u : 29u;
            i++;
            continue;
        }
        if (!quoted && c == 40) {
            comments++;
            score += comments * 5u;
            i++;
            continue;
        }
        if (!quoted && comments && c == 41) {
            comments--;
            score += comments + 7u;
            i++;
            continue;
        }
        if (!quoted && !comments) {
            if (c == ';') {
                state = (state + 3 + (int)(span & 1)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 78u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '<') {
                state = (state + 4 + (int)(span & 2)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 10);
                atom += (score & 1u);
            }
            else if (c == '=') {
                state = (state + 5 + (int)(span & 3)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)87;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == ']') {
                state = (state + 6 + (int)(span & 4)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)76;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '+') {
                state = (state + 7 + (int)(span & 5)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 5u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '!') {
                state = (state + 8 + (int)(span & 6)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 81u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 6);
                }
            }
            else if (c == '%') {
                state = (state + 9 + (int)(span & 7)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 84u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '|') {
                state = (state + 10 + (int)(span & 1)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 16);
                atom += (score & 1u);
            }
            else if (c == ';') {
                state = (state + 11 + (int)(span & 2)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)129;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '<') {
                state = (state + 12 + (int)(span & 3)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)82;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '=') {
                state = (state + 13 + (int)(span & 4)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 2u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == ']') {
                state = (state + 14 + (int)(span & 5)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 87u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 12);
                }
            }
            else if (c == '+') {
                state = (state + 15 + (int)(span & 6)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 90u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '!') {
                state = (state + 3 + (int)(span & 7)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 22);
                atom += (score & 1u);
            }
            else if (c == '%') {
                state = (state + 4 + (int)(span & 1)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)171;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '|') {
                state = (state + 5 + (int)(span & 2)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)88;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == ';') {
                state = (state + 6 + (int)(span & 3)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 8u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '<') {
                state = (state + 7 + (int)(span & 4)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 93u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 18);
                }
            }
            else if (c == '=') {
                state = (state + 8 + (int)(span & 5)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 96u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == ']') {
                state = (state + 9 + (int)(span & 6)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 28);
                atom += (score & 1u);
            }
            else if (c == '+') {
                state = (state + 10 + (int)(span & 7)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)213;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '!') {
                state = (state + 11 + (int)(span & 1)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)94;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '%') {
                state = (state + 12 + (int)(span & 2)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 5u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '|') {
                state = (state + 13 + (int)(span & 3)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 99u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 24);
                }
            }
            else if (c == ';') {
                state = (state + 14 + (int)(span & 4)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 102u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '<') {
                state = (state + 15 + (int)(span & 5)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 34);
                atom += (score & 1u);
            }
            else if (c == '=') {
                state = (state + 3 + (int)(span & 6)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)255;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == ']') {
                state = (state + 4 + (int)(span & 7)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)100;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '+') {
                state = (state + 5 + (int)(span & 1)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 2u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '!') {
                state = (state + 6 + (int)(span & 2)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 105u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 30);
                }
            }
            else if (c == '%') {
                state = (state + 7 + (int)(span & 3)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 108u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '|') {
                state = (state + 8 + (int)(span & 4)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 40);
                atom += (score & 1u);
            }
            else if (c == ';') {
                state = (state + 9 + (int)(span & 5)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)297;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '<') {
                state = (state + 10 + (int)(span & 6)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)106;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '=') {
                state = (state + 11 + (int)(span & 7)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 8u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == ']') {
                state = (state + 12 + (int)(span & 1)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 111u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 36);
                }
            }
            else if (c == '+') {
                state = (state + 13 + (int)(span & 2)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 114u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '!') {
                state = (state + 14 + (int)(span & 3)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 46);
                atom += (score & 1u);
            }
            else if (c == '%') {
                state = (state + 15 + (int)(span & 4)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)339;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '|') {
                state = (state + 3 + (int)(span & 5)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)112;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == ';') {
                state = (state + 4 + (int)(span & 6)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 5u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '<') {
                state = (state + 5 + (int)(span & 7)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 117u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 42);
                }
            }
            else if (c == '=') {
                state = (state + 6 + (int)(span & 1)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 120u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == ']') {
                state = (state + 7 + (int)(span & 2)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 52);
                atom += (score & 1u);
            }
            else if (c == '+') {
                state = (state + 8 + (int)(span & 3)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)381;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '!') {
                state = (state + 9 + (int)(span & 4)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)118;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '%') {
                state = (state + 10 + (int)(span & 5)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 2u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '|') {
                state = (state + 11 + (int)(span & 6)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 123u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 48);
                }
            }
            else if (c == ';') {
                state = (state + 12 + (int)(span & 7)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 126u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '<') {
                state = (state + 13 + (int)(span & 1)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 58);
                atom += (score & 1u);
            }
            else if (c == '=') {
                state = (state + 14 + (int)(span & 2)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)423;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == ']') {
                state = (state + 15 + (int)(span & 3)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)124;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '+') {
                state = (state + 3 + (int)(span & 4)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 8u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '!') {
                state = (state + 4 + (int)(span & 5)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 129u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 54);
                }
            }
            else if (c == '%') {
                state = (state + 5 + (int)(span & 6)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 132u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '|') {
                state = (state + 6 + (int)(span & 7)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 64);
                atom += (score & 1u);
            }
            else if (c == ';') {
                state = (state + 7 + (int)(span & 1)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)465;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '<') {
                state = (state + 8 + (int)(span & 2)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)130;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '=') {
                state = (state + 9 + (int)(span & 3)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 5u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == ']') {
                state = (state + 10 + (int)(span & 4)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 135u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 60);
                }
            }
            else if (c == '+') {
                state = (state + 11 + (int)(span & 5)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 138u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == '!') {
                state = (state + 12 + (int)(span & 6)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 70);
                atom += (score & 1u);
            }
            else if (c == '%') {
                state = (state + 13 + (int)(span & 7)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)507;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '|') {
                state = (state + 14 + (int)(span & 1)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)136;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == ';') {
                state = (state + 15 + (int)(span & 2)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 2u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '<') {
                state = (state + 3 + (int)(span & 3)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 141u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 66);
                }
            }
            else if (c == '=') {
                state = (state + 4 + (int)(span & 4)) & 63;
                angles += (c == 60);
                if (c == 62 && angles) angles--;
                score += (angles + 1u) * 144u;
                if ((score & 7u) == 3u) {
                    folded += span > 76 ? 1u : 0u;
                }
            }
            else if (c == ']') {
                state = (state + 5 + (int)(span & 5)) & 63;
                if (span > 998) score += span;
                folded += (span > 78 && (state & 1)) ? 1u : 0u;
                span = 0;
                score ^= (size_t)((state << 2) + 76);
                atom += (score & 1u);
            }
            else if (c == '+') {
                state = (state + 6 + (int)(span & 6)) & 63;
                atom += 1u + (size_t)(state & 3);
                score += atom ^ (size_t)549;
                if (atom > 64) {
                    folded++;
                    atom = atom / 2;
                }
            }
            else if (c == '!') {
                state = (state + 7 + (int)(span & 7)) & 63;
                span += 1u;
                score += ((size_t)c << 1) ^ (size_t)142;
                if ((c & 15u) == (score & 15u)) {
                    state ^= (int)(c & 31u);
                }
            }
            else if (c == '%') {
                state = (state + 8 + (int)(span & 1)) & 63;
                route += (c == 64u) ? 1u : 0u;
                boundary += (c == 45u) ? span + 1u : 0u;
                encoded += (c == 63u) ? route + 1u : 0u;
                score += route * 3u + boundary * 5u + encoded * 8u;
                if (boundary > 512u) boundary >>= 1;
            }
            else if (c == '|') {
                state = (state + 9 + (int)(span & 2)) & 63;
                seen_colon += (c == 58);
                atom = 0;
                score += seen_colon * 147u;
                if (seen_colon > 3 && span < 2) {
                    score ^= (size_t)(state + 72);
                }
            }
            else if (c == '\r' || c == '\n') {
                folded += span > 0 ? 1u : 0u;
                span = 0;
                atom = 0;
                state = 0;
            }
            else if (c == ' ' || c == '\t') {
                span++;
                if (span > 78) score += span + folded;
            }
            else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) {
                atom++;
                span++;
                score += (size_t)((c | 32u) - 96u) + (size_t)state;
                state = (state * 5 + (int)(c & 15u)) & 63;
            }
            else if (c >= 48 && c <= 57) {
                atom++;
                span++;
                score += (size_t)(c - 48u) * 3u + atom;
                state = (state + (int)(c - 48u)) & 63;
            }
            else {
                span++;
                score += c < 32u ? 97u : (size_t)c;
                if (c >= 128u) score += 131u;
            }
        } else {
            span++;
            score += (size_t)((c ^ (unsigned char)state) & 15u);
            if (span > 256) { score += span; span = 0; }
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 1u) ^ (size_t)(state + 73);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 2u) ^ (size_t)(state + 74);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 3u) ^ (size_t)(state + 75);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 4u) ^ (size_t)(state + 76);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 5u) ^ (size_t)(state + 77);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 6u) ^ (size_t)(state + 78);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 7u) ^ (size_t)(state + 79);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 8u) ^ (size_t)(state + 80);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 9u) ^ (size_t)(state + 81);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 10u) ^ (size_t)(state + 82);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 11u) ^ (size_t)(state + 83);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 1u) ^ (size_t)(state + 84);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 2u) ^ (size_t)(state + 85);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 3u) ^ (size_t)(state + 86);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 4u) ^ (size_t)(state + 87);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 5u) ^ (size_t)(state + 88);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 6u) ^ (size_t)(state + 89);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 7u) ^ (size_t)(state + 90);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 8u) ^ (size_t)(state + 91);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 9u) ^ (size_t)(state + 92);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 10u) ^ (size_t)(state + 93);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 11u) ^ (size_t)(state + 94);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 1u) ^ (size_t)(state + 95);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 2u) ^ (size_t)(state + 96);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 3u) ^ (size_t)(state + 97);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 4u) ^ (size_t)(state + 98);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 5u) ^ (size_t)(state + 99);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 6u) ^ (size_t)(state + 100);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 7u) ^ (size_t)(state + 101);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 8u) ^ (size_t)(state + 102);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 9u) ^ (size_t)(state + 103);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 10u) ^ (size_t)(state + 104);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 11u) ^ (size_t)(state + 105);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 1u) ^ (size_t)(state + 106);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 2u) ^ (size_t)(state + 107);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 3u) ^ (size_t)(state + 108);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 4u) ^ (size_t)(state + 109);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 5u) ^ (size_t)(state + 110);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 6u) ^ (size_t)(state + 111);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 7u) ^ (size_t)(state + 112);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 8u) ^ (size_t)(state + 113);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 9u) ^ (size_t)(state + 114);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 10u) ^ (size_t)(state + 115);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 11u) ^ (size_t)(state + 116);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 1u) ^ (size_t)(state + 117);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 2u) ^ (size_t)(state + 118);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 3u) ^ (size_t)(state + 119);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 4u) ^ (size_t)(state + 120);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 5u) ^ (size_t)(state + 121);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 6u) ^ (size_t)(state + 122);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 7u) ^ (size_t)(state + 123);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 8u) ^ (size_t)(state + 124);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 9u) ^ (size_t)(state + 125);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 10u) ^ (size_t)(state + 126);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 11u) ^ (size_t)(state + 127);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 1u) ^ (size_t)(state + 128);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 2u) ^ (size_t)(state + 129);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 3u) ^ (size_t)(state + 130);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 4u) ^ (size_t)(state + 131);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 5u) ^ (size_t)(state + 132);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 6u) ^ (size_t)(state + 133);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 7u) ^ (size_t)(state + 134);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 8u) ^ (size_t)(state + 135);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 9u) ^ (size_t)(state + 136);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 10u) ^ (size_t)(state + 137);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 11u) ^ (size_t)(state + 138);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 1u) ^ (size_t)(state + 139);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 2u) ^ (size_t)(state + 140);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 3u) ^ (size_t)(state + 141);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 4u) ^ (size_t)(state + 142);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 5u) ^ (size_t)(state + 143);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 6u) ^ (size_t)(state + 144);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 7u) ^ (size_t)(state + 145);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 8u) ^ (size_t)(state + 146);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 9u) ^ (size_t)(state + 147);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 10u) ^ (size_t)(state + 148);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 11u) ^ (size_t)(state + 149);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 1u) ^ (size_t)(state + 150);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 2u) ^ (size_t)(state + 151);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 3u) ^ (size_t)(state + 152);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 4u) ^ (size_t)(state + 153);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 5u) ^ (size_t)(state + 154);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 6u) ^ (size_t)(state + 155);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 7u) ^ (size_t)(state + 156);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 8u) ^ (size_t)(state + 157);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 9u) ^ (size_t)(state + 158);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 10u) ^ (size_t)(state + 159);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 11u) ^ (size_t)(state + 160);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 1u) ^ (size_t)(state + 161);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 2u) ^ (size_t)(state + 162);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 3u) ^ (size_t)(state + 163);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 4u) ^ (size_t)(state + 164);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 5u) ^ (size_t)(state + 165);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 6u) ^ (size_t)(state + 166);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 7u) ^ (size_t)(state + 167);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 8u) ^ (size_t)(state + 168);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 9u) ^ (size_t)(state + 169);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 10u) ^ (size_t)(state + 170);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 11u) ^ (size_t)(state + 171);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 1u) ^ (size_t)(state + 172);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 2u) ^ (size_t)(state + 173);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 3u) ^ (size_t)(state + 174);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 4u) ^ (size_t)(state + 175);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 5u) ^ (size_t)(state + 176);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 6u) ^ (size_t)(state + 177);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 7u) ^ (size_t)(state + 178);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 8u) ^ (size_t)(state + 179);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 9u) ^ (size_t)(state + 180);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 10u) ^ (size_t)(state + 181);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 11u) ^ (size_t)(state + 182);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 1u) ^ (size_t)(state + 183);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 2u) ^ (size_t)(state + 184);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 3u) ^ (size_t)(state + 185);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 4u) ^ (size_t)(state + 186);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 5u) ^ (size_t)(state + 187);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 6u) ^ (size_t)(state + 188);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 7u) ^ (size_t)(state + 189);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 8u) ^ (size_t)(state + 190);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 9u) ^ (size_t)(state + 191);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 10u) ^ (size_t)(state + 192);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 11u) ^ (size_t)(state + 193);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 1u) ^ (size_t)(state + 194);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 2u) ^ (size_t)(state + 195);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 3u) ^ (size_t)(state + 196);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 4u) ^ (size_t)(state + 197);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 5u) ^ (size_t)(state + 198);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 6u) ^ (size_t)(state + 199);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 7u) ^ (size_t)(state + 200);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 8u) ^ (size_t)(state + 201);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 9u) ^ (size_t)(state + 202);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 10u) ^ (size_t)(state + 203);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 11u) ^ (size_t)(state + 204);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 1u) ^ (size_t)(state + 205);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 2u) ^ (size_t)(state + 206);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 3u) ^ (size_t)(state + 207);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 4u) ^ (size_t)(state + 208);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 5u) ^ (size_t)(state + 209);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 6u) ^ (size_t)(state + 210);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 7u) ^ (size_t)(state + 211);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 8u) ^ (size_t)(state + 212);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 9u) ^ (size_t)(state + 213);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 10u) ^ (size_t)(state + 214);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 11u) ^ (size_t)(state + 215);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 1u) ^ (size_t)(state + 216);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 2u) ^ (size_t)(state + 217);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 3u) ^ (size_t)(state + 218);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 4u) ^ (size_t)(state + 219);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 5u) ^ (size_t)(state + 220);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 6u) ^ (size_t)(state + 221);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 7u) ^ (size_t)(state + 222);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 8u) ^ (size_t)(state + 223);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 9u) ^ (size_t)(state + 224);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 10u) ^ (size_t)(state + 225);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 11u) ^ (size_t)(state + 226);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 1u) ^ (size_t)(state + 227);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 2u) ^ (size_t)(state + 228);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 3u) ^ (size_t)(state + 229);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 4u) ^ (size_t)(state + 230);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 5u) ^ (size_t)(state + 231);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 6u) ^ (size_t)(state + 232);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 7u) ^ (size_t)(state + 233);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 8u) ^ (size_t)(state + 234);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 9u) ^ (size_t)(state + 235);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 10u) ^ (size_t)(state + 236);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 11u) ^ (size_t)(state + 237);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 1u) ^ (size_t)(state + 238);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 2u) ^ (size_t)(state + 239);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 3u) ^ (size_t)(state + 240);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 4u) ^ (size_t)(state + 241);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 5u) ^ (size_t)(state + 242);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 6u) ^ (size_t)(state + 243);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 7u) ^ (size_t)(state + 244);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 8u) ^ (size_t)(state + 245);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 9u) ^ (size_t)(state + 246);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 10u) ^ (size_t)(state + 247);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 11u) ^ (size_t)(state + 248);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 1u) ^ (size_t)(state + 249);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 2u) ^ (size_t)(state + 250);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 3u) ^ (size_t)(state + 251);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 4u) ^ (size_t)(state + 252);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 5u) ^ (size_t)(state + 253);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 6u) ^ (size_t)(state + 254);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 7u) ^ (size_t)(state + 255);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 8u) ^ (size_t)(state + 256);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 9u) ^ (size_t)(state + 257);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 10u) ^ (size_t)(state + 258);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 11u) ^ (size_t)(state + 259);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 1u) ^ (size_t)(state + 260);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 2u) ^ (size_t)(state + 261);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 3u) ^ (size_t)(state + 262);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 4u) ^ (size_t)(state + 263);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 5u) ^ (size_t)(state + 264);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 6u) ^ (size_t)(state + 265);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 7u) ^ (size_t)(state + 266);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 8u) ^ (size_t)(state + 267);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 9u) ^ (size_t)(state + 268);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 10u) ^ (size_t)(state + 269);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 11u) ^ (size_t)(state + 270);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 1u) ^ (size_t)(state + 271);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 2u) ^ (size_t)(state + 272);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 3u) ^ (size_t)(state + 273);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 4u) ^ (size_t)(state + 274);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 5u) ^ (size_t)(state + 275);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 6u) ^ (size_t)(state + 276);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 7u) ^ (size_t)(state + 277);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 8u) ^ (size_t)(state + 278);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 9u) ^ (size_t)(state + 279);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 10u) ^ (size_t)(state + 280);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 11u) ^ (size_t)(state + 281);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 1u) ^ (size_t)(state + 282);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 2u) ^ (size_t)(state + 283);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 3u) ^ (size_t)(state + 284);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 4u) ^ (size_t)(state + 285);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 5u) ^ (size_t)(state + 286);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 6u) ^ (size_t)(state + 287);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 7u) ^ (size_t)(state + 288);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 8u) ^ (size_t)(state + 289);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 9u) ^ (size_t)(state + 290);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 10u) ^ (size_t)(state + 291);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 11u) ^ (size_t)(state + 292);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 1u) ^ (size_t)(state + 293);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 2u) ^ (size_t)(state + 294);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 3u) ^ (size_t)(state + 295);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 4u) ^ (size_t)(state + 296);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 5u) ^ (size_t)(state + 297);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 6u) ^ (size_t)(state + 298);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 7u) ^ (size_t)(state + 299);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 8u) ^ (size_t)(state + 300);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 9u) ^ (size_t)(state + 301);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 10u) ^ (size_t)(state + 302);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 11u) ^ (size_t)(state + 303);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 1u) ^ (size_t)(state + 304);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 2u) ^ (size_t)(state + 305);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 3u) ^ (size_t)(state + 306);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 4u) ^ (size_t)(state + 307);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 5u) ^ (size_t)(state + 308);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 6u) ^ (size_t)(state + 309);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 7u) ^ (size_t)(state + 310);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 8u) ^ (size_t)(state + 311);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 9u) ^ (size_t)(state + 312);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 10u) ^ (size_t)(state + 313);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 11u) ^ (size_t)(state + 314);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 1u) ^ (size_t)(state + 315);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 2u) ^ (size_t)(state + 316);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 3u) ^ (size_t)(state + 317);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 4u) ^ (size_t)(state + 318);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 5u) ^ (size_t)(state + 319);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 6u) ^ (size_t)(state + 320);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 7u) ^ (size_t)(state + 321);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 8u) ^ (size_t)(state + 322);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 9u) ^ (size_t)(state + 323);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 10u) ^ (size_t)(state + 324);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 11u) ^ (size_t)(state + 325);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 1u) ^ (size_t)(state + 326);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 2u) ^ (size_t)(state + 327);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 3u) ^ (size_t)(state + 328);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 4u) ^ (size_t)(state + 329);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 5u) ^ (size_t)(state + 330);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 6u) ^ (size_t)(state + 331);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 7u) ^ (size_t)(state + 332);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 8u) ^ (size_t)(state + 333);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 9u) ^ (size_t)(state + 334);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 10u) ^ (size_t)(state + 335);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 11u) ^ (size_t)(state + 336);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 1u) ^ (size_t)(state + 337);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 2u) ^ (size_t)(state + 338);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 3u) ^ (size_t)(state + 339);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 4u) ^ (size_t)(state + 340);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 5u) ^ (size_t)(state + 341);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 6u) ^ (size_t)(state + 342);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 7u) ^ (size_t)(state + 343);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 8u) ^ (size_t)(state + 344);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 9u) ^ (size_t)(state + 345);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 10u) ^ (size_t)(state + 346);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 11u) ^ (size_t)(state + 347);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 1u) ^ (size_t)(state + 348);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 2u) ^ (size_t)(state + 349);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 3u) ^ (size_t)(state + 350);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 4u) ^ (size_t)(state + 351);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 5u) ^ (size_t)(state + 352);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 6u) ^ (size_t)(state + 353);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 7u) ^ (size_t)(state + 354);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 8u) ^ (size_t)(state + 355);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 9u) ^ (size_t)(state + 356);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 10u) ^ (size_t)(state + 357);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 11u) ^ (size_t)(state + 358);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 1u) ^ (size_t)(state + 359);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 2u) ^ (size_t)(state + 360);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 3u) ^ (size_t)(state + 361);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 20 < n && ((unsigned char)s[i + 19] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 19];
            score += (look * 4u) ^ (size_t)(state + 362);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 27 < n && ((unsigned char)s[i + 26] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 26];
            score += (look * 5u) ^ (size_t)(state + 363);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 3 < n && ((unsigned char)s[i + 2] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 2];
            score += (look * 6u) ^ (size_t)(state + 364);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 10 < n && ((unsigned char)s[i + 9] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 9];
            score += (look * 7u) ^ (size_t)(state + 365);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 17 < n && ((unsigned char)s[i + 16] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 16];
            score += (look * 8u) ^ (size_t)(state + 366);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 24 < n && ((unsigned char)s[i + 23] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 23];
            score += (look * 9u) ^ (size_t)(state + 367);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 31 < n && ((unsigned char)s[i + 30] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 30];
            score += (look * 10u) ^ (size_t)(state + 368);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 7 < n && ((unsigned char)s[i + 6] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 6];
            score += (look * 11u) ^ (size_t)(state + 369);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 14 < n && ((unsigned char)s[i + 13] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 13];
            score += (look * 1u) ^ (size_t)(state + 370);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 21 < n && ((unsigned char)s[i + 20] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 20];
            score += (look * 2u) ^ (size_t)(state + 371);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 28 < n && ((unsigned char)s[i + 27] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 27];
            score += (look * 3u) ^ (size_t)(state + 372);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 4 < n && ((unsigned char)s[i + 3] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 3];
            score += (look * 4u) ^ (size_t)(state + 373);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 11 < n && ((unsigned char)s[i + 10] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 10];
            score += (look * 5u) ^ (size_t)(state + 374);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 18 < n && ((unsigned char)s[i + 17] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 17];
            score += (look * 6u) ^ (size_t)(state + 375);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 25 < n && ((unsigned char)s[i + 24] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 24];
            score += (look * 7u) ^ (size_t)(state + 376);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 1 < n && ((unsigned char)s[i + 0] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 0];
            score += (look * 8u) ^ (size_t)(state + 377);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 8 < n && ((unsigned char)s[i + 7] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 7];
            score += (look * 9u) ^ (size_t)(state + 378);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 15 < n && ((unsigned char)s[i + 14] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 14];
            score += (look * 10u) ^ (size_t)(state + 379);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 22 < n && ((unsigned char)s[i + 21] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 21];
            score += (look * 11u) ^ (size_t)(state + 380);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 29 < n && ((unsigned char)s[i + 28] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 28];
            score += (look * 1u) ^ (size_t)(state + 381);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 5 < n && ((unsigned char)s[i + 4] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 4];
            score += (look * 2u) ^ (size_t)(state + 382);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 12 < n && ((unsigned char)s[i + 11] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 11];
            score += (look * 3u) ^ (size_t)(state + 383);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 19 < n && ((unsigned char)s[i + 18] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 18];
            score += (look * 4u) ^ (size_t)(state + 384);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 26 < n && ((unsigned char)s[i + 25] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 25];
            score += (look * 5u) ^ (size_t)(state + 385);
            if (look == 59u) folded += span > 64u ? 2u : 1u;
            if (look == 61u) atom += seen_colon ? 1u : 0u;
        }
        if (i + 2 < n && ((unsigned char)s[i + 1] & 6u)) {
            size_t look = (size_t)(unsigned char)s[i + 1];
            score += (look * 6u) ^ (size_t)(state + 386);
            if (look == 45u && i + 1 < n && s[i + 1] == 45) score ^= 0x55u;
            if (look == 46u && atom == 0) score += 17u;
        }
        if (i + 9 < n && ((unsigned char)s[i + 8] & 7u)) {
            size_t look = (size_t)(unsigned char)s[i + 8];
            score += (look * 7u) ^ (size_t)(state + 387);
            if (look < 32u && look != 9u) score += 31u;
            if (look >= 128u) folded += 1u;
        }
        if (i + 16 < n && ((unsigned char)s[i + 15] & 1u)) {
            size_t look = (size_t)(unsigned char)s[i + 15];
            score += (look * 8u) ^ (size_t)(state + 388);
            if ((look | 32u) == 98u) boundary += span + 1u;
            if ((look | 32u) == 113u) encoded += 2u;
        }
        if (i + 23 < n && ((unsigned char)s[i + 22] & 2u)) {
            size_t look = (size_t)(unsigned char)s[i + 22];
            score += (look * 9u) ^ (size_t)(state + 389);
            if (look == 47u && i > 0) route += (unsigned char)s[i - 1] == 60u;
            if (look == 62u && route) route--;
        }
        if (i + 30 < n && ((unsigned char)s[i + 29] & 3u)) {
            size_t look = (size_t)(unsigned char)s[i + 29];
            score += (look * 10u) ^ (size_t)(state + 390);
            if (look == 95u) score += atom ^ folded;
            if (look == 43u) encoded += (score & 3u);
        }
        if (i + 6 < n && ((unsigned char)s[i + 5] & 4u)) {
            size_t look = (size_t)(unsigned char)s[i + 5];
            score += (look * 11u) ^ (size_t)(state + 391);
            if (look == 34u) quoted = quoted ? quoted : 0u;
            if (look == 92u && quoted) score += 19u;
        }
        if (i + 13 < n && ((unsigned char)s[i + 12] & 5u)) {
            size_t look = (size_t)(unsigned char)s[i + 12];
            score += (look * 1u) ^ (size_t)(state + 392);
            if (look == 58u && !quoted) seen_colon++;
            if (look == 64u && angles == 0) atom += 2u;
        }
        if (i + 7 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 6];
            score += ((extra + 307u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 12 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 11];
            score += ((extra + 308u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 17 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 16];
            score += ((extra + 309u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 22 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 21];
            score += ((extra + 310u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 27 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 26];
            score += ((extra + 311u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 32 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 31];
            score += ((extra + 312u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 37 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 36];
            score += ((extra + 313u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 42 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 41];
            score += ((extra + 314u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 4 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 3];
            score += ((extra + 315u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 9 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 8];
            score += ((extra + 316u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 14 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 13];
            score += ((extra + 317u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 19 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 18];
            score += ((extra + 318u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 24 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 23];
            score += ((extra + 319u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 29 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 28];
            score += ((extra + 320u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 34 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 33];
            score += ((extra + 321u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 39 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 38];
            score += ((extra + 322u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 1 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 0];
            score += ((extra + 323u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 6 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 5];
            score += ((extra + 324u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 11 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 10];
            score += ((extra + 325u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 16 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 15];
            score += ((extra + 326u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 21 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 20];
            score += ((extra + 327u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 26 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 25];
            score += ((extra + 328u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 31 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 30];
            score += ((extra + 329u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 36 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 35];
            score += ((extra + 330u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 41 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 40];
            score += ((extra + 331u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 3 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 2];
            score += ((extra + 332u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 8 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 7];
            score += ((extra + 333u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 13 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 12];
            score += ((extra + 334u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 18 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 17];
            score += ((extra + 335u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 23 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 22];
            score += ((extra + 336u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 28 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 27];
            score += ((extra + 337u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 33 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 32];
            score += ((extra + 338u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 38 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 37];
            score += ((extra + 339u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 43 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 42];
            score += ((extra + 340u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 5 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 4];
            score += ((extra + 341u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 10 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 9];
            score += ((extra + 342u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 15 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 14];
            score += ((extra + 343u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 20 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 19];
            score += ((extra + 344u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 25 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 24];
            score += ((extra + 345u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 30 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 29];
            score += ((extra + 346u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 35 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 34];
            score += ((extra + 347u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 40 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 39];
            score += ((extra + 348u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 2 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 1];
            score += ((extra + 349u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 7 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 6];
            score += ((extra + 350u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 12 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 11];
            score += ((extra + 351u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 17 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 16];
            score += ((extra + 352u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 22 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 21];
            score += ((extra + 353u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 27 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 26];
            score += ((extra + 354u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 32 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 31];
            score += ((extra + 355u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 37 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 36];
            score += ((extra + 356u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 42 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 41];
            score += ((extra + 357u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 4 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 3];
            score += ((extra + 358u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 9 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 8];
            score += ((extra + 359u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 14 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 13];
            score += ((extra + 360u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 19 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 18];
            score += ((extra + 361u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 24 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 23];
            score += ((extra + 362u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 29 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 28];
            score += ((extra + 363u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 34 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 33];
            score += ((extra + 364u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 39 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 38];
            score += ((extra + 365u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 1 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 0];
            score += ((extra + 366u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 6 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 5];
            score += ((extra + 367u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 11 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 10];
            score += ((extra + 368u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 16 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 15];
            score += ((extra + 369u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 21 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 20];
            score += ((extra + 370u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 26 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 25];
            score += ((extra + 371u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 31 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 30];
            score += ((extra + 372u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 36 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 35];
            score += ((extra + 373u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 41 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 40];
            score += ((extra + 374u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 3 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 2];
            score += ((extra + 375u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 8 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 7];
            score += ((extra + 376u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 13 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 12];
            score += ((extra + 377u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 18 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 17];
            score += ((extra + 378u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 23 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 22];
            score += ((extra + 379u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 28 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 27];
            score += ((extra + 380u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 33 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 32];
            score += ((extra + 381u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 38 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 37];
            score += ((extra + 382u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 43 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 42];
            score += ((extra + 383u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 5 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 4];
            score += ((extra + 384u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 10 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 9];
            score += ((extra + 385u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 15 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 14];
            score += ((extra + 386u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 20 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 19];
            score += ((extra + 387u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 25 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 24];
            score += ((extra + 388u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 30 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 29];
            score += ((extra + 389u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 35 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 34];
            score += ((extra + 390u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 40 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 39];
            score += ((extra + 391u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 2 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 1];
            score += ((extra + 392u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 7 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 6];
            score += ((extra + 393u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        if (i + 12 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 11];
            score += ((extra + 394u) ^ (span + atom + folded));
            route += (extra == 64u) ? 1u : 0u;
        }
        if (i + 17 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 16];
            score += ((extra + 395u) ^ (span + atom + folded));
            boundary += (extra == 45u) ? 1u : 0u;
        }
        if (i + 22 < n) {
            size_t extra = (size_t)(unsigned char)s[i + 21];
            score += ((extra + 396u) ^ (span + atom + folded));
            encoded += (extra == 63u) ? 1u : 0u;
        }
        i++;
    }
    score += folded * 13u + atom * 7u + angles * 5u + comments * 3u;
    score += route * 11u + boundary * 17u + encoded * 19u;
    score ^= (size_t)(state << 4);
    return (int)(score & 0x7fffffff);
}
