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

# Plot data
def plot_data(dataframe):
    if dataframe is not None:
        x = dataframe.iloc[:, 0]  # Assuming first column is x-axis data
        y = dataframe.iloc[:, 1]  # Assuming second column is y-axis data

        plt.plot(x, y)
        plt.xlabel('X Label')
        plt.ylabel('Y Label')
        plt.title('Data Plot')
        plt.grid(True)
        plt.show()

# Main function
def main():
    csv_filename = 'example.csv'
    data = load_data_from_csv(csv_filename)

    # Grouping the data by 'Sched_Policy' and 'Transform', then calculate standard deviation
    grouped_std_dev = data.groupby(['Sched_Policy', 'Transform', 'Resolution'])['Execution_time'].std().reset_index()
    print(grouped_std_dev);

    grouped_std_dev


    if data is not None:
        # Define the range of rows you want to plot
        start_row = 2
        end_row = 10

        # Slice the DataFrame to get the desired range of rows
        sliced_data = data.iloc[start_row:end_row+1]

        # Sort the sliced data by the "Number" column
        sliced_data = sliced_data.sort_values(by='Number')

        # Plot Data
        plt.figure(figsize=(10, 6))
        plt.plot(sliced_data['Number'], sliced_data['Execution_time'], marker='o', linestyle='-')
        plt.title('Number vs. Execution Time')
        plt.xlabel('Number')
        plt.ylabel('Execution Time')
        plt.grid(True)
        plt.show()

if __name__ == "__main__":
    main()
