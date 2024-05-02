import matplotlib.pyplot as plt
import pandas as pd
import subprocess
import os

def run_experiment(messages, corruption, time, loss, window, path, output_file):
    exec_str = ["./run_experiments","-p",path,"-m",messages,"-t",time,"-c",corruption,"-w",window,"-l",loss,"-o",output_file]
    subprocess.run(exec_str)


def plot_and_save_data(data, x_label, y_label, title, graph_filename, data_filename):
    x_abt = data.get('abt').get('x')
    y_abt = data.get('abt').get('y')
    x_gbn = data.get('gbn').get('x')
    y_gbn = data.get('gbn').get('y')
    x_sr = data.get('sr').get('x')
    y_sr = data.get('sr').get('y')

    fig, ax = plt.subplots()
    ax.plot(x_abt,y_abt, label="Alternating Bit")
    ax.plot(x_gbn,y_gbn, label="Go-Back-N")
    ax.plot(x_sr,y_sr, label="Selective Repeat")

    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.legend()

    fig.savefig('./graphs/'+graph_filename)
    df = pd.DataFrame({'X_ABT':x_abt, 'Y_ABT':y_abt, 'X_GBN':x_gbn, 'Y_GBN':y_gbn, 'X_SR':x_sr, 'Y_SR':y_sr})
    df.to_csv('./outputs/'+data_filename, index=False)

    

def get_average_throughput(output_file):
    df = pd.read_csv(output_file)
    average_tp = df['Throughput'].mean()
    return average_tp

# Ensure the output directory exists
os.makedirs('./graphs', exist_ok=True)
os.makedirs('./outputs', exist_ok=True)

# Experiment 1: Varying loss probability with different window sizes
protocols = ['abt', 'gbn', 'sr']
window_sizes = [10, 50]
loss_probs = [0.1, 0.2, 0.4, 0.6, 0.8]

for window_size in window_sizes:
    data = {protocol: {'x': [], 'y': []} for protocol in protocols}
    for loss_prob in loss_probs:
        for protocol in protocols:
            output_file = './outputs/exp1_{}_l{}_w{}.csv'.format(protocol, str(int(loss_prob * 100)), str(window_size))
            path = "../protocols/"+protocol
            run_experiment("1000","0.2","50",str(loss_prob),str(window_size),path,output_file)
            avg_throughput = get_average_throughput(output_file)
            data.get(protocol).get('x').append(loss_prob)
            data.get(protocol).get('y').append(avg_throughput)
    x_label = "Loss Probability"
    y_label = "Average Throughput"
    title = "Experiment 1, Window Size " + str(window_size)
    filename = "ex1_w" + str(window_size)
    plot_and_save_data(data, x_label, y_label, title, filename+".png", filename+".csv")

# Experiment 2: Varying window size with fixed loss probabilities
loss_probs = [0.2, 0.5, 0.8]
window_sizes = [10, 50, 100, 200, 500]

for loss_prob in loss_probs:
    data = {protocol: {'x': [], 'y': []} for protocol in protocols}
    for window_size in window_sizes:
        for protocol in protocols:
            output_file = './outputs/exp2_{}_l{}_w{}.csv'.format(protocol, str(int(loss_prob * 100)), str(window_size))
            path = "../protocols/"+protocol
            run_experiment("1000","0.2","50",str(loss_prob),str(window_size),path,output_file)
            avg_throughput = get_average_throughput(output_file)
            data.get(protocol).get('x').append(window_size)
            data.get(protocol).get('y').append(avg_throughput)
    x_label = "Window Size"
    y_label = "Average Throughput"
    title = "Experiment 2, Loss Probability " + str(loss_prob)
    filename = "ex2_l" + str(loss_prob)
    plot_and_save_data(data, x_label, y_label, title, filename+".png", filename+".csv")