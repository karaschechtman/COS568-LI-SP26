import pandas as pd
import matplotlib.pyplot as plt
import os
import glob
from collections import defaultdict


def _parse_dataset_and_workload(filename):
    base = os.path.basename(filename)

    dataset = base
    suffix = "_public_uint64_ops_2M_0.000000rq_0.500000nl_"
    if suffix in base:
        dataset = base.split(suffix, 1)[0]
    elif "_ops_2M_" in base:
        dataset = base.split("_ops_2M_", 1)[0]

    if "0.100000i" in base:
        workload_label = "10% Inserts"
    elif "0.900000i" in base:
        workload_label = "90% Inserts"
    else:
        workload_label = os.path.splitext(base)[0]

    return dataset, workload_label


def _best_results_for_csv(csv_file):
    df = pd.read_csv(csv_file)
    df['avg_throughput'] = df[['mixed_throughput_mops1', 'mixed_throughput_mops2', 'mixed_throughput_mops3']].mean(axis=1)

    best_results = {}
    for index_name in df['index_name'].unique():
        index_df = df[df['index_name'] == index_name]
        best_row = index_df.loc[index_df['avg_throughput'].idxmax()]

        best_results[index_name] = {
            'avg_throughput': best_row['avg_throughput'],
            'index_size': best_row['index_size_bytes'],
            'search_method': best_row.get('search_method', 'N/A'),
            'value': best_row.get('value', 'N/A'),
            'percent': best_row.get('percent', 'N/A'),
        }

    return best_results


def _plot_dataset(dataset_name, workload_results, output_path):
    workload_labels = [label for label in ["10% Inserts", "90% Inserts"] if label in workload_results]
    if not workload_labels:
        print(f"Skipping {dataset_name}: no workload files found")
        return

    all_indices = sorted({index_name for results in workload_results.values() for index_name in results.keys()})

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    x_pos = range(len(all_indices))
    bar_width = 0.35 if len(workload_labels) > 1 else 0.6

    throughput_data = {label: [] for label in workload_labels}
    size_data = {label: [] for label in workload_labels}

    for index_name in all_indices:
        for label in workload_labels:
            if index_name in workload_results[label]:
                throughput_data[label].append(workload_results[label][index_name]['avg_throughput'])
                size_data[label].append(workload_results[label][index_name]['index_size'] / 1e9)
            else:
                throughput_data[label].append(0)
                size_data[label].append(0)

    for i, label in enumerate(workload_labels):
        ax1.bar([x + i * bar_width for x in x_pos], throughput_data[label], bar_width, label=label)

    ax1.set_xlabel('Index')
    ax1.set_ylabel('Throughput (Mops/s)')
    ax1.set_title(f'{dataset_name}: Best Average Throughput by Index')
    ax1.set_xticks([x + bar_width / 2 for x in x_pos])
    ax1.set_xticklabels(all_indices)
    ax1.legend()
    ax1.grid(axis='y', alpha=0.3)

    for i, label in enumerate(workload_labels):
        ax2.bar([x + i * bar_width for x in x_pos], size_data[label], bar_width, label=label)

    ax2.set_xlabel('Index')
    ax2.set_ylabel('Index Size (GB)')
    ax2.set_title(f'{dataset_name}: Index Size for Best Configuration')
    ax2.set_xticks([x + bar_width / 2 for x in x_pos])
    ax2.set_xticklabels(all_indices)
    ax2.legend()
    ax2.grid(axis='y', alpha=0.3)

    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close(fig)


def _print_dataset_summary(dataset_name, workload_results):
    workload_labels = [label for label in ["10% Inserts", "90% Inserts"] if label in workload_results]
    all_indices = sorted({index_name for results in workload_results.values() for index_name in results.keys()})

    print(f"\n=== {dataset_name} ===")
    print(f"{'Index':<15} {'Workload':<15} {'Throughput':<15} {'Size (GB)':<12} {'Search Method':<20} {'Value':<12} {'Percent':<12}")
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
                index_col = index_name if first_row else ""
                print(f"{index_col:<15} {label:<15} {throughput:<15} {size_gb:<12} {search_method:<20} {value:<12} {percent:<12}")
                first_row = False
            else:
                index_col = index_name if first_row else ""
                print(f"{index_col:<15} {label:<15} {'N/A':<15} {'N/A':<12} {'N/A':<20} {'N/A':<12} {'N/A':<12}")
                first_row = False

def analyze_workloads():
    """
    Read mixed workload results, compute best throughput for each index,
    and generate one comparison figure per dataset.
    """

    results_dir = "results"
    csv_files = sorted(glob.glob(os.path.join(results_dir, "*_mix_results_table.csv")))

    datasets = defaultdict(dict)
    for csv_file in csv_files:
        dataset_name, workload_label = _parse_dataset_and_workload(csv_file)
        datasets[dataset_name][workload_label] = _best_results_for_csv(csv_file)

    for dataset_name in sorted(datasets.keys()):
        workload_results = datasets[dataset_name]
        output_path = os.path.join(results_dir, f"workload_comparison_{dataset_name}.png")
        _plot_dataset(dataset_name, workload_results, output_path)
        _print_dataset_summary(dataset_name, workload_results)
        print(f"Chart saved to {output_path}")

if __name__ == "__main__":
    analyze_workloads()
