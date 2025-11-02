import math

#import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('results.csv', names=['benchmark', 'mean', 'median', 'stddev', 'samples'])

calibration = ['00Calibration', '01SyscallCalibration', '01ThreadCalibration']
measurements = [[f'00Fn_v_i{n}' for n in range(11)], ['01SyscallMeasurement'], ['01ThreadMeasurement']]
display_names = [[f'{n} ints' for n in range(11)], ['getpid()'], ['Counting Semaphore']]
titles = ['Function Call', 'System Call', 'Thread Ping-Pong']


# Determine grid size
cols = 3  # you can adjust this
rows = 3

fig, axes = plt.subplots(rows, cols, figsize=(5 * cols, 4 * rows), squeeze=False)

for i in range(len(calibration)):
    c_data = df[df['benchmark'] == calibration[i]]
    for j, m in enumerate(measurements[i]):
        data = df[df['benchmark'] == m]
        axes[0][i].plot(data['mean'].to_numpy() - c_data['mean'].to_numpy(), label=display_names[i][j])
        axes[1][i].plot(data['median'].to_numpy() - c_data['median'].to_numpy(), label=display_names[i][j])
    axes[0][i].set_title('Mean: ' + titles[i])
    axes[1][i].set_title('Median: ' + titles[i])

    ax = axes[2][i]
    ax.plot(c_data['mean'].to_numpy(), label='Mean')
    ax.plot(c_data['median'].to_numpy(), label='Median')
    ax.plot(c_data['stddev'].to_numpy(), label='Stddev')
    ax.set_title('Calibration: ' + titles[i])
    for j in range(3):
        ax = axes[j][i]
        ax.set_xlabel('Run')
        ax.set_ylabel('Cycles')
        ax.grid()
        ax.legend()

plt.tight_layout()
plt.show()
