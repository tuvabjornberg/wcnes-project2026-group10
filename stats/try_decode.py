from functions import *
import numpy as np

sent = (
    "00 00 13 5d 1f 43 1b 45 25 6f 21 fe 0f 7e",
    "00 0c 20 6b 2b b0 24 4b 26 55 1a 56 22 e8",
    "00 18 14 20 1f 30 25 19 1a 14 23 a8 23 8c",
    "00 24 19 e2 17 c2 25 4e 1f 4c 27 a4 31 bf",
    "00 30 1c 08 23 fc 17 c2 22 1b 1c e4 25 de",
    "00 3c 20 56 1c 27 1e 24 17 07 1a 80 27 e6",
    "00 48 2a 99 2b 6b 1e 0d 1a 32 22 76 19 1f",
    "00 54 25 a3 2f 9e 25 88 22 96 1e 88 20 eb",
    "00 60 16 6d 12 10 22 7b 2c 67 11 f6 1f a4",
    "00 6c 12 ef 22 26 29 73 31 88 26 2d 1e 06",
    "00 78 20 2d 2b f3 19 7d 20 ab 19 6b 0e 36",
    "00 84 23 eb 12 4a 23 19 14 e7 29 81 1d cf",
    "00 90 20 4f 20 cd 31 6b 1d d7 14 34 1f f0",
    "00 9c 31 bc 2d 44 20 a6 12 42 21 f3 20 51",
    "00 a8 11 0e 25 a4 20 c1 23 31 21 d2 13 7a",
    "00 b4 1f b9 1d 5e 20 ef 2b 95 25 60 27 37",
    "00 c0 1f 9a 2c 71 21 2c 21 7f 1e f7 24 92",
    "00 cc 21 17 2d 98 26 9a 14 08 1e 5a 1c c6",
    "00 d8 2d d7 22 e9 1c e0 11 66 28 f0 1e 22",
    "00 e4 2d 5b 20 66 27 a2 25 65 1d 8c 23 54",
)
raw = (
    "00 00 00 e0 39 98 5b ff 83 47 55 3d b4 54 ad cc 7f 76 c8 e0 ff 7f 00 9e 06",
    "00 40 18 00 c8 54 67 d5 0c a0 aa ca 54 55 b3 4c ab 25 3d cc 5a 99 cc 12 0f",
    "00 c0 f2 a0 3a 00 32 ff 03 c0 d3 ca 30 0f d2 83 ea e0 c9 2c a5 9e 4c 78 09",
    "00 80 2a c3 3c 64 f0 b4 43 26 dc ca e0 55 ff 43 58 45 cb a8 a4 07 cf bf 0a",
    "00 00 c0 13 3e 2c 01 9e 4c f8 4f 3b 64 c2 99 4c f5 10 3e a8 f0 ad 0c de 0c",
    "00 40 d8 03 c8 cc 5a e1 03 2d 83 3f a8 32 b4 03 0d 20 3d 00 96 b4 cc 0c 0f",
    "00 c0 52 25 cd 30 99 d5 4c 75 86 3f 98 01 d2 43 c6 93 c9 cc 68 cc c3 ff 00",
    "00 80 aa d5 ca 78 a4 ff 0c 9e d9 ca 2c 97 99 cc 8c 89 3f 2c 97 80 4c 15 0f",
    "00 00 60 36 3b 98 67 99 03 e0 90 c9 54 69 e1 0c 6d 76 38 cc fe ff 83 4a 0a",
    "00 40 78 96 39 fc f1 99 cc 2c c3 cc 78 68 07 cf 72 39 cb 98 33 f8 c3 0c 00",
    "00 c0 92 06 c8 98 33 d5 8c e7 cf 3c 98 69 80 4c 55 ca 3c 54 67 78 c0 cc 03",
    "00 80 6a e9 c9 54 f1 99 83 54 e5 c9 30 0f aa 03 0d cf cc 1c 96 e6 c3 3f 0c",
    "00 00 80 09 c8 fc 55 80 8c 39 7c f0 54 67 e6 03 cd ac 3a a8 3c ff 03 e0 0f",
    "00 40 98 79 f0 84 ab e6 8c 4a 05 c8 cc a4 99 43 46 75 c8 78 fe 80 cc a1 05",
    "00 c0 52 7a 38 e0 01 ad 8c 4a 0a c8 1c c2 9e cc c1 73 c8 64 cc 9e 83 94 06",
    "00 80 aa fa 3f 30 ab e6 03 be 05 c8 fc f1 d5 4c 8b d9 ca 00 66 b4 0c cd 03",
    "00 00 20 fc 3f 48 99 e1 cc 81 76 c8 84 33 87 cc 9f 86 3f d0 fe aa 4c 86 09",
    "00 40 38 7c c8 d0 0e e6 cc 92 39 cb 48 99 aa c3 12 80 3f 48 5b e1 c3 2c 0c",
    "00 c0 d2 6c ce d0 cc 99 0c 13 1f 3e 00 f0 87 c3 6c b6 cc 00 fe f8 43 26 03",
    "00 80 0a 6f ce 54 5b 80 cc 6c 46 cb 64 a4 ad 4c 6b 66 3e 84 97 9e 8c aa 05",
)
ENCSIZE = 25

decoded = (
    hamming_decode_buffer(
        np.bitwise_xor(
            np.frombuffer(bytes.fromhex(raw_msg), dtype=np.uint8),
            1 << np.random.randint(0, 8, size=ENCSIZE),
        )
    )
    for raw_msg in raw
)
for sent, input, (msg, errs) in zip(sent, raw, decoded):
    print("Errs:", errs)
    print("Enco:", input)
    as_str = bytes(msg).hex(" ")
    print("Sent:", sent)
    print("Recv:", as_str)
    print()
