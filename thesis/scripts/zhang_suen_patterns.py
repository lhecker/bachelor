#!/usr/bin/env python3


def generate_patterns(second_pass=None):
    for i in range(0, 512):
        p0 = [(i >> 8 - j) & 1 for j in range(0, 3)]
        p1 = [(i >> 8 - j) & 1 for j in range(3, 6)]
        p2 = [(i >> 8 - j) & 1 for j in range(6, 9)]

        if p1[1] == 0:
            continue

        white_pixels = sum(p0) + sum(p1) + sum(p2)
        if white_pixels < 3 or white_pixels > 7:
            continue

        transitions_black_white = (p0[0] ^ p0[1]) + \
                                  (p0[1] ^ p0[2]) + \
                                  (p0[2] ^ p1[2]) + \
                                  (p1[2] ^ p2[2]) + \
                                  (p2[2] ^ p2[1]) + \
                                  (p2[1] ^ p2[0]) + \
                                  (p2[0] ^ p1[0]) + \
                                  (p1[0] ^ p0[0])
        if transitions_black_white != 2:
            continue

        if second_pass is True:
            if (p0[1] + p1[0] + p1[2]) == 3 or (p0[1] + p1[0] + p2[1]) == 3:
                continue
        elif second_pass is False:
            if (p0[1] + p1[2] + p2[1]) == 3 or (p1[0] + p1[2] + p2[1]) == 3:
                continue

        yield [p0, p1, p2]


for pattern in generate_patterns(second_pass=None):
    for p in pattern:
        print(''.join([str(v) for v in p]))
    print()
