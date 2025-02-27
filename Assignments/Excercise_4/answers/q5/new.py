import matplotlib.pyplot as plt
import pandas as pd

# Load data from CSV file
def load_data_from_csv(filename):
    try:
        df = pd.read_csv(filename)
        return df
    except FileNotFoundError:
        print(f"File {filename} not found.")
        return None
    except Exception as e:
        print(f"An error occurred while loading data: {e}")
        return None

# Main function
def main():
    csv_filename = './example.csv'
    data = load_data_from_csv(csv_filename)

    temp = 144
    if data is not None:
        if temp != 0:
            ranges = [(temp, temp+8), (temp+9, temp+17), (temp+18, temp+26), (temp+27, temp+35)]
            fig, axs = plt.subplots(nrows=2, ncols=2, figsize=(12, 8))
        else:
            ranges = [(temp, temp+8), (temp+9, temp+17)]
            fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(12, 8))

        for i, ax in enumerate(axs.flat):
            start, end = ranges[i]
            sliced_data = data.iloc[start:end+1]
            sliced_data = sliced_data.sort_values(by='Number')
            resolution = sliced_data['Resolution'].iloc[0]
            sched_method = sliced_data['Sched_Policy'].iloc[0]
            Transformation_method = sliced_data['Transform'].iloc[0]
            ax.plot(sliced_data['Number'], sliced_data['Execution_time'], marker='o', linestyle='-', label=f"Range {start}-{end}")
            if sched_method == 1:
                ax.set_title(f'Resolution - {resolution}')
            else:
                ax.set_title(f'Resolution - {resolution} ')
            ax.set_xlabel('Number')
            ax.set_ylabel('Execution Time')
            ax.grid(True)
            ax.legend()

        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    main()
