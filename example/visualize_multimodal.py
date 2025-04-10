from matplotlib import pyplot as plt


import csv

x = []
y_ecg = []
y_ppg = []

FILTER_CUTOFF = 150

with open("ppg_filt.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        x.append(float(row[0]))
        y_ppg.append(float(row[1]))
        y_ecg.append(float(row[2]))

ecg_ts = []
ECG_TR = 0
ECG_BL = 1
with open("ecg_a.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        hb = [int(row[ECG_TR]), int(row[ECG_BL])]
        ecg_ts.append(hb)
        # y.append(float(row[1])

ppg_ts = []
PPG_TS = 0
PPG_TM = 1
PPG_TE = 2
PPG_TD = 3
with open("ppg_a.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        hb = [int(row[PPG_TS]), int(row[PPG_TM]), int(row[PPG_TE]), int(row[PPG_TD])]
        ppg_ts.append(hb)
        # y.append(float(row[1]))

multimodal_ts = []
MM_PPG = 1
MM_ECG = 0
with open("mm_a.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        hb = [int(row[MM_ECG]), int(row[MM_PPG])]
        multimodal_ts.append(hb)
        print(multimodal_ts)
        # y.append(float(row[1]))

# Create a figure and a set of subplots (1 row, 2 columns)
fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(10,5))

# Plot data on the first subplot (index 0)
axes[0].plot(x[FILTER_CUTOFF:], y_ecg[FILTER_CUTOFF:], color='blue')
axes[0].set_title('ECG')
axes[0].set_xlabel('Time [ms]')
axes[0].set_xlim(65200, 68500)
axes[0].set_ylabel('Y-axis')


flip = True
for hb_collect in ecg_ts:
    mini_x = []
    mini_y = []
    ts = hb_collect[0]
    if ts != 0:
        pass
        mini_x.append(ts)
        mini_y.append(y_ecg[x.index(ts)])
        mini_x.append(ts)
        mini_y.append(hb_collect[1])
    if flip:
        axes[0].scatter(mini_x, mini_y, c="blue")
    else:
        axes[0].scatter(mini_x, mini_y, c="red")
    flip ^= True
    # if ts != 0:
    #     plt.scatter(ts, y[x.index(ts)])

# Plot data on the second subplot (index 1)
axes[1].plot(x[FILTER_CUTOFF:], y_ppg[FILTER_CUTOFF:], color='red')
axes[1].set_title('PPG')
axes[1].set_xlabel('Time [ms]')
axes[1].set_ylabel('Y-axis')
axes[1].set_xlim(65200, 68500)

flip = True
for hb_collect in ppg_ts:
    mini_x = []
    mini_y = []
    for ts in hb_collect:
        if ts != 0:
            pass
            mini_x.append(ts)
            mini_y.append(y_ppg[x.index(ts)])
    if flip:
        axes[1].scatter(mini_x, mini_y, c="blue")
    else:
        axes[1].scatter(mini_x, mini_y, c="red")
    flip ^= True
    # if ts != 0:
    #     plt.scatter(ts, y[x.index(ts)])


flip = True
for hb_collect in multimodal_ts:
    x_ppg = hb_collect[1]
    x_ecg = hb_collect[0]
    if x_ecg == 0 or x_ppg == 0:
        continue

    y_ppg_i = y_ppg[x.index(x_ppg)]
    y_ecg_i = y_ecg[x.index(x_ecg)]
    if flip:
        axes[1].scatter(x_ppg, y_ppg_i, c="green")
        axes[0].scatter(x_ecg, y_ecg_i, c="green")
    else:
        axes[1].scatter(x_ppg, y_ppg_i, c="black")
        axes[0].scatter(x_ecg, y_ecg_i, c="black")

    flip ^= True


# Adjust layout to prevent overlapping of titles/labels
plt.tight_layout()

# Show the plot
plt.show()


