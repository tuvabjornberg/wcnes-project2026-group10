from typing import Any

import matplotlib.pyplot as plt
from numpy import dtype, float64
from numpy._typing._array_like import NDArray
from scipy.optimize import differential_evolution
import scipy as sp

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
files = [
    ("base  50cm", "./baseline2-50cm.csv"),
    ("base 100cm", "./baseline2-100cm.csv"),
    ("base 150cm", "./baseline2-150cm.csv"),
    ("base 200cm", "./baseline2-200cm.csv"),
    ("base 250cm", "./baseline2-250cm.csv"),
    ("opt   50cm", "./opt1-reliability-50cm.csv"),
    ("opt  100cm", "./opt1-reliability-100cm.csv"),
    ("opt  150cm", "./opt1-reliability-150cm.csv"),
    ("opt  200cm", "./opt1-reliability-200cm.csv"),
    ("opt  250cm", "./opt1-reliability-250cm.csv"),
]


def modular_linear_fit(t, y, mod=256):
    """
    Fits (k*t + c) mod 256 to noisy data with missing values.
    """
    # 1. Remove missing values (NaNs)
    mask = ~np.isnan(y)
    t_data = t[mask]
    y_data = y[mask]

    # 2. Define the Circular Loss function
    # We convert the error to radians. On a circle, the distance
    # between 255 and 0 is small. 1 - cos(error) captures this.
    def loss(params):
        k, c = params
        y_pred = (k * t_data + c) % mod

        # Transform difference to radians: [0, mod] -> [0, 2*pi]
        error_rad = (2 * np.pi / mod) * (y_data - y_pred)

        # Minimize 1 - cos(theta). Result is 0 when values match,
        # and 2 when they are 180 degrees apart (max error).
        return np.sum(1 - np.cos(error_rad))

    # 3. Global Optimization
    # We provide bounds for k (slope) and c (intercept).
    # If you know your slope is positive or within a range, tighten these.
    bounds = [(0, 10), (0, mod)]

    result = differential_evolution(loss, bounds)

    return result.x  # Returns [k, c]


fig, axes = plt.subplots((len(files) + 1) // 2, 2, figsize=(16, 12))
axes = axes.flatten()
ymax = 0

for ax, (dist, filename) in zip(axes, files):
    df = readfile(filename)

    result = sp.stats.mode(df.payload.map(lambda x: len(x)))
    df = df[df.payload.apply(lambda x: len(x) == result.mode)]
    df.reset_index(inplace=True)

    if len(df) == 0:
        print("Warning: no packets")

    times = np.array(
        [
            np.timedelta64(p_rx - df.time_rx[0], "s").astype(np.float64)
            for p_rx in df.time_rx
        ]
    )

    k, m = modular_linear_fit(times, df.seq)

    t = np.linspace(np.min(times), np.max(times), 1000)
    unwrapped = df.seq + 256 * ((k * times + m) // 256)

    ax.set_title(dist)
    ax.scatter(times, df.seq, label="unwrapped")
    ax.plot(t, np.mod(k * t + m, 256), "--", label="fitted")
    ax.legend()

    last = int(unwrapped[len(df) - 1])
    est_last = int(k * times[len(df) - 1] + m)

    print(f"{dist}: {last} & {100*(200/last):.2f} \\%")
    # print(f"est {dist}: {est_last} & {200/est_last:.2f}")

# for ax in axes:
#     ax.set_ylim(0, ymax)j

plt.tight_layout()
plt.show()
