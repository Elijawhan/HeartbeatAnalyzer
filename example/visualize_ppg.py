from matplotlib import pyplot as plt


import csv

x = []
y = []

with open("ppg_filt.csv") as file:
    csv_reader = csv.reader(file)
    for row in csv_reader:
        x.append(float(row[0]))
        y.append(float(row[1]))

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

plt.plot(x[150:], y[150:])

flip = True
for hb_collect in ppg_ts:
    mini_x = []
    mini_y = []
    for ts in hb_collect:
        if ts != 0:
            pass
            mini_x.append(ts)
            mini_y.append(y[x.index(ts)])
    if flip:
        plt.scatter(mini_x, mini_y, c="blue")
    else:
        plt.scatter(mini_x, mini_y, c="red")
    flip ^= True
    # if ts != 0:
    #     plt.scatter(ts, y[x.index(ts)])
print(len(ppg_ts))
print(ppg_ts)
plt.show()
