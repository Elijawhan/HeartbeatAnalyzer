from matplotlib import pyplot as plt


import csv

x = []
y = []

with open("ppg_filt.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        x.append(float(row[0]))
        y.append(float(row[2]))

ecg_ts = []
ECG_TR = 0
ECG_BL = 1
with open("ecg_a.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        hb = [int(row[ECG_TR]), int(row[ECG_BL])]
        ecg_ts.append(hb)
        # y.append(float(row[1]))

plt.plot(x[150:], y[150:])

flip = True
for hb_collect in ecg_ts:
    mini_x = []
    mini_y = []
    ts = hb_collect[0]
    if ts != 0:
        pass
        mini_x.append(ts)
        mini_y.append(y[x.index(ts)])
        mini_x.append(ts)
        mini_y.append(hb_collect[1])
    if flip:
        plt.scatter(mini_x, mini_y, c="blue")
    else:
        plt.scatter(mini_x, mini_y, c="red")
    flip ^= True
    # if ts != 0:
    #     plt.scatter(ts, y[x.index(ts)])
print(len(ecg_ts))
print(ecg_ts)
plt.show()
