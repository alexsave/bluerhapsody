import numpy as np
import matplotlib.pyplot as plt
import os

def plot_data_from_file(file_path):
    if not os.path.exists(file_path):
        print(f"Error: File not found at {file_path}")
        return

    try:
        data = np.loadtxt(file_path)

        if data.ndim == 1:
             print("Error: Data file appears to contain only one column.")
             return

        #x_values = np.log10(data[:, 0])
        x_values = data[:, 0]
        y_values = data[:, 1]

    except Exception as e:
        print(f"\nAn error occurred while reading the file. Check your format.")
        print(f"Details: {e}")
        return

    plt.figure(figsize=(10, 6)) # Set a nice size for the plot
    plt.plot(x_values, y_values, label='Data Points', marker='o', linestyle='-', markersize=3)

    # Adding labels and title improves readability
    plt.title('Plot of Y vs X from File Data')
    plt.xlabel('First Value (X)')
    plt.ylabel('Second Value (Y)')
    
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend() 
    plt.tight_layout() 

    # Display the generated plot
    plt.show()

FILE_NAME = "f"

plot_data_from_file(FILE_NAME)

