import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

from functions import *  # import functions from the .py file

from pylab import rcParams

rcParams["figure.figsize"] = 16, 4

PAYLOADSIZE = 14

if PAYLOADSIZE % 2 != 0:
    print("Alarm! the payload size is not even.")
NUM_16RND = (
    PAYLOADSIZE - 2
) // 2  # how many 16 bits random number included in each frame
MAX_SEQ = 256  # (decimal) maximum seq number defined by the length of the seq, the length of seq is 1B

files = {
    50: "baseline2-50cm",
    100: "baseline2-100cm",
    150: "baseline2-150cm",
    200: "baseline2-200cm",
    250: "baseline2-250cm",
}

fig, axes = plt.subplots(3, 2, figsize=(16, 12))
axes = axes.flatten()
ymax = 0

for ax, (dist, filename) in zip(axes, files.items()):
    df = readfile(filename + ".csv")

    df = df[df.payload.apply(lambda x: len(x) == ((PAYLOADSIZE) * 3 - 1))]
    df.reset_index(inplace=True)

    bit_errors = (
        compute_ber_packet(row, PACKET_LEN=NUM_16RND * 2) for _, row in df.iterrows()
    )

    ber = [e / l for e, l in bit_errors]
    ax.hist(ber, bins=15)
    ax.set_title(f"{dist} cm", fontsize=14)
    ax.set_xlabel("BER [%]", fontsize=12)
    ax.xaxis.set_major_formatter(mtick.PercentFormatter(xmax=1))
    ax.grid(True)
    ymax = max(ymax, ax.get_ylim()[1])


for ax in axes:
    ax.set_ylim(0, ymax)

plt.tight_layout()
plt.show()
