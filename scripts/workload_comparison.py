import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

def analyze_workloads():
    """
    Read two CSV files with mixed workload results, compute best throughput 
    for each index, and generate comparison bar charts.
    """
    
    # Find all CSVs in results/ directory with mixed workload results
    results_dir = "results"
    csv_files = sorted(glob.glob(os.path.join(results_dir, "*_mix_results_table.csv")))
    
    # Dictionary to store best results per workload
    workload_results = {}
    workload_labels = []
    
    # Process each CSV file
    for csv_file in csv_files[:2]:  # Only use first 2 files
        df = pd.read_csv(csv_file)
        
        # Extract workload label from filename (e.g., "0.100000i" or "0.900000i")
        filename = os.path.basename(csv_file)
        if "0.100000i" in filename:
            workload_label = "10% Inserts"
        elif "0.900000i" in filename:
            workload_label = "90% Inserts"
        else:
            workload_label = os.path.splitext(filename)[0]
        
        workload_labels.append(workload_label)
        
        # Calculate average throughput for each row
        df['avg_throughput'] = df[['mixed_throughput_mops1', 'mixed_throughput_mops2', 'mixed_throughput_mops3']].mean(axis=1)
        
        # For each index, find the row with best average throughput
        best_results = {}
        for index_name in df['index_name'].unique():
            index_df = df[df['index_name'] == index_name]
            best_row = index_df.loc[index_df['avg_throughput'].idxmax()]
            
            best_results[index_name] = {
                'avg_throughput': best_row['avg_throughput'],
                'index_size': best_row['index_size_bytes'],
                'search_method': best_row.get('search_method', 'N/A'),
                'value': best_row.get('value', 'N/A'),
                'percent': best_row.get('percent', 'N/A')
            }
        
        workload_results[workload_label] = best_results
    
    # Prepare data for plotting
    all_indices = set()
    for results in workload_results.values():
        all_indices.update(results.keys())
    all_indices = sorted(list(all_indices))
    
    # Create figure with two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Prepare data for throughput comparison
    x_pos = range(len(all_indices))
    bar_width = 0.35
    
    throughput_data = {label: [] for label in workload_labels}
    size_data = {label: [] for label in workload_labels}
    
    for index_name in all_indices:
        for label in workload_labels:
            if index_name in workload_results[label]:
                throughput_data[label].append(workload_results[label][index_name]['avg_throughput'])
                size_data[label].append(workload_results[label][index_name]['index_size'] / 1e9)  # Convert to GB
            else:
                throughput_data[label].append(0)
                size_data[label].append(0)
    
    # Plot 1: Throughput comparison
    for i, label in enumerate(workload_labels):
        ax1.bar([x + i*bar_width for x in x_pos], throughput_data[label], bar_width, label=label)
    
    ax1.set_xlabel('Index')
    ax1.set_ylabel('Throughput (Mops/s)')
    ax1.set_title('Best Average Throughput by Index')
    ax1.set_xticks([x + bar_width/2 for x in x_pos])
    ax1.set_xticklabels(all_indices)
    ax1.legend()
    ax1.grid(axis='y', alpha=0.3)
    
    # Plot 2: Index size comparison
    for i, label in enumerate(workload_labels):
        ax2.bar([x + i*bar_width for x in x_pos], size_data[label], bar_width, label=label)
    
    ax2.set_xlabel('Index')
    ax2.set_ylabel('Index Size (GB)')
    ax2.set_title('Index Size for Best Configuration')
    ax2.set_xticks([x + bar_width/2 for x in x_pos])
    ax2.set_xticklabels(all_indices)
    ax2.legend()
    ax2.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('results/workload_comparison.png', dpi=300, bbox_inches='tight')
    print("Chart saved to results/workload_comparison.png")
    
    # Print unified summary table
    print("\n=== Best Configuration Results ===")
    print(f"\n{'Index':<15} {'Workload':<15} {'Throughput':<15} {'Size (GB)':<12} {'Search Method':<20} {'Value':<12} {'Percent':<12}")
    print("-" * 115)
    
    for index_name in all_indices:
        first_row = True
        for label in workload_labels:
            if index_name in workload_results[label]:
                result = workload_results[label][index_name]
                throughput = f"{result['avg_throughput']:.4f}"
                size_gb = f"{result['index_size'] / 1e9:.3f}"
                search_method = str(result['search_method'])
                value = str(result['value'])
                percent = str(result['percent']) if result['percent'] is not None else 'NaN'
                
                # Only print index name on first row for that index
                index_col = index_name if first_row else ""
                
                print(f"{index_col:<15} {label:<15} {throughput:<15} {size_gb:<12} {search_method:<20} {value:<12} {percent:<12}")
                first_row = False
            else:
                index_col = index_name if first_row else ""
                print(f"{index_col:<15} {label:<15} {'N/A':<15} {'N/A':<12} {'N/A':<20} {'N/A':<12} {'N/A':<12}")
                first_row = False

if __name__ == "__main__":
    analyze_workloads()
