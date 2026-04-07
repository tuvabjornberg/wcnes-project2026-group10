# Move file to /stats to run...

import matplotlib.pyplot as plt
from functions import * # import functions from the .py file

from pylab import rcParams
rcParams["figure.figsize"] = 16, 4

PAYLOADSIZE = 14

if PAYLOADSIZE % 2 != 0:
    print("Alarm! the payload size is not even.")
NUM_16RND = (PAYLOADSIZE-2)//2 # how many 16 bits random number included in each frame
MAX_SEQ = 256 # (decimal) maximum seq number defined by the length of the seq, the length of seq is 1B

files = {
    50: "baseline-exp/test50cm",
    100: "baseline-exp/test100cm",
    150: "baseline-exp/test150cm",
    200: "baseline-exp/test200cm",
    250: "baseline-exp/test250cm",
    275: "baseline-exp/test275cm",
}

fig, axes = plt.subplots(3, 2, figsize=(16, 12)) 
axes = axes.flatten()  

for ax, (dist, filename) in zip(axes, files.items()):
    df = readfile(filename + ".csv")
    
    ber_values = [
        compute_ber_packet(row, PACKET_LEN=NUM_16RND*2)[0]
        for _, row in df.iterrows()
    ]
    
    ax.scatter(range(len(df)), ber_values, s=6, color='black')
    ax.set_title(f"{dist} cm", fontsize=14)
    ax.set_ylabel("BER [%]", fontsize=12)
    ax.set_xlabel("Seq. Number", fontsize=12)
    ax.grid(True)

plt.tight_layout()
plt.show()