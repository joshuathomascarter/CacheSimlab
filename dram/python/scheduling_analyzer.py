#!/usr/bin/env python3
"""
Day 3 Scheduling Analysis Tool
Analyzes scheduler performance, BLP, fairness, and latency distributions
"""

import json
import sys
from pathlib import Path
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np
from typing import Dict, List, Tuple

class SchedulingAnalyzer:
    def __init__(self):
        self.latencies = []
        self.per_thread_latencies = {}
        self.blp_history = []
        self.row_hit_history = []
        
    def analyze_latency_distribution(self, latencies: List[int]) -> Dict:
        """Calculate latency percentiles and statistics"""
        if not latencies:
            return {}
        
        sorted_lat = sorted(latencies)
        n = len(sorted_lat)
        
        return {
            'min': sorted_lat[0],
            'max': sorted_lat[-1],
            'mean': np.mean(sorted_lat),
            'median': sorted_lat[n // 2],
            'p95': sorted_lat[int(n * 0.95)],
            'p99': sorted_lat[int(n * 0.99)],
            'p999': sorted_lat[int(n * 0.999)] if n > 1000 else sorted_lat[-1],
            'std': np.std(sorted_lat)
        }
    
    def calculate_fairness_index(self, per_thread_service: Dict[int, int]) -> float:
        """Calculate Jain's Fairness Index"""
        if not per_thread_service:
            return 0.0
        
        values = list(per_thread_service.values())
        sum_val = sum(values)
        sum_squared = sum(v * v for v in values)
        n = len(values)
        
        if sum_squared == 0:
            return 0.0
        
        return (sum_val * sum_val) / (n * sum_squared)
    
    def compare_schedulers(self, results: Dict[str, Dict]) -> None:
        """Compare different scheduling policies"""
        print("\n" + "="*80)
        print("SCHEDULER COMPARISON")
        print("="*80)
        
        print(f"\n{'Policy':<15} {'Avg Lat':<12} {'P95':<12} {'P99':<12} {'BLP':<10} {'Hit Rate':<10}")
        print("-" * 80)
        
        for policy, stats in results.items():
            print(f"{policy:<15} "
                  f"{stats.get('average_latency', 0):<12.1f} "
                  f"{stats.get('p95', 0):<12.1f} "
                  f"{stats.get('p99', 0):<12.1f} "
                  f"{stats.get('blp', 0):<10.2f} "
                  f"{stats.get('row_hit_rate', 0)*100:<10.1f}%")
        
        print()
    
    def plot_comprehensive_analysis(self, 
                                    latencies: List[int],
                                    blp_history: List[float],
                                    per_thread_latencies: Dict[int, List[int]],
                                    row_hit_history: List[bool],
                                    policy_name: str = "Scheduler",
                                    output_file: str = "scheduling_analysis.png"):
        """Create comprehensive visualization"""
        
        fig = plt.figure(figsize=(16, 12))
        gs = gridspec.GridSpec(3, 3, figure=fig, hspace=0.3, wspace=0.3)
        
        # 1. Latency Distribution (CDF)
        ax1 = fig.add_subplot(gs[0, :2])
        sorted_lat = sorted(latencies)
        cdf = np.arange(1, len(sorted_lat) + 1) / len(sorted_lat)
        ax1.plot(sorted_lat, cdf, linewidth=2, color='#2E86AB')
        ax1.set_xlabel('Latency (cycles)', fontsize=12)
        ax1.set_ylabel('CDF', fontsize=12)
        ax1.set_title(f'{policy_name}: Latency Distribution (CDF)', fontsize=14, fontweight='bold')
        ax1.grid(True, alpha=0.3)
        
        # Add percentile markers
        stats = self.analyze_latency_distribution(latencies)
        for p, label in [(0.95, 'P95'), (0.99, 'P99')]:
            idx = int(len(sorted_lat) * p)
            ax1.axvline(sorted_lat[idx], color='red', linestyle='--', alpha=0.7)
            ax1.text(sorted_lat[idx], 0.5, f'{label}\n{sorted_lat[idx]:.0f}', 
                    ha='right', fontsize=10)
        
        # 2. Latency Statistics
        ax2 = fig.add_subplot(gs[0, 2])
        ax2.axis('off')
        stats_text = f"""
LATENCY STATISTICS

Mean:    {stats['mean']:.1f} cycles
Median:  {stats['median']:.1f} cycles
Std Dev: {stats['std']:.1f} cycles

Min:     {stats['min']:.0f} cycles
Max:     {stats['max']:.0f} cycles

P95:     {stats['p95']:.0f} cycles
P99:     {stats['p99']:.0f} cycles
P99.9:   {stats['p999']:.0f} cycles
        """
        ax2.text(0.1, 0.9, stats_text, fontsize=11, family='monospace',
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        # 3. Bank-Level Parallelism Over Time
        ax3 = fig.add_subplot(gs[1, :])
        if blp_history:
            time_points = range(len(blp_history))
            ax3.plot(time_points, blp_history, linewidth=1, color='#A23B72', alpha=0.7)
            
            # Moving average
            window = min(100, len(blp_history) // 10)
            if window > 1:
                moving_avg = np.convolve(blp_history, np.ones(window)/window, mode='valid')
                ax3.plot(range(len(moving_avg)), moving_avg, linewidth=2, 
                        color='#F18F01', label=f'{window}-cycle MA')
            
            ax3.set_xlabel('Cycle', fontsize=12)
            ax3.set_ylabel('Active Banks', fontsize=12)
            ax3.set_title('Bank-Level Parallelism Over Time', fontsize=14, fontweight='bold')
            ax3.legend()
            ax3.grid(True, alpha=0.3)
            
            avg_blp = np.mean(blp_history)
            ax3.axhline(avg_blp, color='green', linestyle='--', 
                       label=f'Average: {avg_blp:.2f}', linewidth=2)
        
        # 4. Per-Thread Latency Distribution (Box Plot)
        ax4 = fig.add_subplot(gs[2, 0])
        if per_thread_latencies:
            thread_ids = sorted(per_thread_latencies.keys())
            data = [per_thread_latencies[tid] for tid in thread_ids]
            
            bp = ax4.boxplot(data, labels=[f'T{tid}' for tid in thread_ids],
                            patch_artist=True)
            
            # Color boxes
            colors = plt.cm.viridis(np.linspace(0, 1, len(thread_ids)))
            for patch, color in zip(bp['boxes'], colors):
                patch.set_facecolor(color)
            
            ax4.set_xlabel('Thread ID', fontsize=12)
            ax4.set_ylabel('Latency (cycles)', fontsize=12)
            ax4.set_title('Per-Thread Latency Distribution', fontsize=14, fontweight='bold')
            ax4.grid(True, alpha=0.3, axis='y')
        
        # 5. Row Buffer Hit Rate Over Time
        ax5 = fig.add_subplot(gs[2, 1])
        if row_hit_history:
            window_size = min(100, len(row_hit_history) // 10)
            if window_size > 1:
                hit_rate = []
                for i in range(len(row_hit_history) - window_size):
                    window = row_hit_history[i:i+window_size]
                    hit_rate.append(sum(window) / window_size * 100)
                
                ax5.plot(hit_rate, linewidth=2, color='#06A77D')
                ax5.set_xlabel('Time Window', fontsize=12)
                ax5.set_ylabel('Hit Rate (%)', fontsize=12)
                ax5.set_title(f'Row Buffer Hit Rate ({window_size}-req window)', 
                             fontsize=14, fontweight='bold')
                ax5.grid(True, alpha=0.3)
                
                overall_hit_rate = sum(row_hit_history) / len(row_hit_history) * 100
                ax5.axhline(overall_hit_rate, color='red', linestyle='--',
                           label=f'Overall: {overall_hit_rate:.1f}%', linewidth=2)
                ax5.legend()
        
        # 6. Fairness Analysis
        ax6 = fig.add_subplot(gs[2, 2])
        if per_thread_latencies:
            thread_ids = sorted(per_thread_latencies.keys())
            avg_latencies = [np.mean(per_thread_latencies[tid]) for tid in thread_ids]
            
            bars = ax6.bar(range(len(thread_ids)), avg_latencies, 
                          color=plt.cm.viridis(np.linspace(0, 1, len(thread_ids))))
            
            ax6.set_xlabel('Thread ID', fontsize=12)
            ax6.set_ylabel('Average Latency (cycles)', fontsize=12)
            ax6.set_title('Fairness: Per-Thread Avg Latency', fontsize=14, fontweight='bold')
            ax6.set_xticks(range(len(thread_ids)))
            ax6.set_xticklabels([f'T{tid}' for tid in thread_ids])
            ax6.grid(True, alpha=0.3, axis='y')
            
            # Calculate and display fairness index
            per_thread_service = {tid: sum(per_thread_latencies[tid]) for tid in thread_ids}
            fairness = self.calculate_fairness_index(per_thread_service)
            ax6.text(0.5, 0.95, f'Fairness Index: {fairness:.3f}',
                    transform=ax6.transAxes, ha='center', va='top',
                    bbox=dict(boxstyle='round', facecolor='yellow', alpha=0.7),
                    fontsize=11, fontweight='bold')
        
        plt.suptitle(f'{policy_name} Performance Analysis', 
                    fontsize=16, fontweight='bold', y=0.995)
        
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"\n✓ Visualization saved to: {output_file}")
        plt.close()
    
    def analyze_blp_vs_mapping(self, results: Dict[str, Dict]) -> None:
        """Analyze BLP for different address mapping schemes"""
        print("\n" + "="*80)
        print("ADDRESS MAPPING vs BANK-LEVEL PARALLELISM")
        print("="*80)
        
        print(f"\n{'Mapping Scheme':<30} {'BLP':<10} {'Active Banks':<15} {'Distribution':<20}")
        print("-" * 80)
        
        for scheme, stats in results.items():
            print(f"{scheme:<30} "
                  f"{stats.get('blp', 0):<10.2f} "
                  f"{stats.get('active_banks', 0):<15} "
                  f"{stats.get('distribution', 'N/A'):<20}")
        
        print()
    
    def generate_report(self, policy_name: str, stats: Dict, output_file: str):
        """Generate detailed markdown report"""
        with open(output_file, 'w') as f:
            f.write(f"# {policy_name} Scheduling Analysis Report\n\n")
            
            f.write("## Executive Summary\n\n")
            f.write(f"- **Average Latency**: {stats.get('mean', 0):.1f} cycles\n")
            f.write(f"- **99th Percentile**: {stats.get('p99', 0):.0f} cycles\n")
            f.write(f"- **Bank-Level Parallelism**: {stats.get('blp', 0):.2f}\n")
            f.write(f"- **Row Buffer Hit Rate**: {stats.get('row_hit_rate', 0)*100:.1f}%\n\n")
            
            f.write("## Latency Distribution\n\n")
            f.write("| Metric | Value |\n")
            f.write("|--------|-------|\n")
            f.write(f"| Minimum | {stats.get('min', 0):.0f} cycles |\n")
            f.write(f"| Maximum | {stats.get('max', 0):.0f} cycles |\n")
            f.write(f"| Mean | {stats.get('mean', 0):.1f} cycles |\n")
            f.write(f"| Median | {stats.get('median', 0):.0f} cycles |\n")
            f.write(f"| Std Dev | {stats.get('std', 0):.1f} cycles |\n")
            f.write(f"| P95 | {stats.get('p95', 0):.0f} cycles |\n")
            f.write(f"| P99 | {stats.get('p99', 0):.0f} cycles |\n")
            f.write(f"| P99.9 | {stats.get('p999', 0):.0f} cycles |\n\n")
            
            f.write("## Key Insights\n\n")
            
            # Latency variance analysis
            if stats.get('p99', 0) > 0:
                variance = (stats.get('p99', 0) - stats.get('mean', 0)) / stats.get('mean', 1)
                f.write(f"- **Latency Variance**: {variance:.2%}\n")
                if variance > 2.0:
                    f.write("  - ⚠️  High tail latency detected. Consider TCM for better QoS.\n")
                else:
                    f.write("  - ✅ Low tail latency. Good for latency-critical workloads.\n")
            
            # BLP analysis
            if stats.get('blp', 0) < 4.0:
                f.write(f"- **BLP < 4**: Consider improving address mapping scheme\n")
            elif stats.get('blp', 0) >= 7.0:
                f.write(f"- **BLP ≥ 7**: Excellent parallelism! Streaming workload optimized.\n")
            
            # Row hit rate
            if stats.get('row_hit_rate', 0) < 0.5:
                f.write(f"- **Low Hit Rate (<50%)**: Random access pattern or poor locality\n")
            elif stats.get('row_hit_rate', 0) >= 0.75:
                f.write(f"- **High Hit Rate (≥75%)**: Excellent spatial locality exploitation\n")
            
            f.write("\n---\n")
            f.write(f"*Generated by Day 3 Scheduling Analyzer*\n")
        
        print(f"✓ Report saved to: {output_file}")

def main():
    analyzer = SchedulingAnalyzer()
    
    # Example usage with synthetic data
    print("\n" + "="*80)
    print("DAY 3 SCHEDULING ANALYSIS TOOL")
    print("="*80)
    
    # Generate sample data for demonstration
    np.random.seed(42)
    
    # Simulate FR-FCFS latencies (low variance, row-hit optimized)
    latencies_fr_fcfs = []
    for _ in range(1000):
        if np.random.random() < 0.78:  # 78% row hits
            latencies_fr_fcfs.append(int(np.random.normal(50, 10)))
        else:  # Row misses
            latencies_fr_fcfs.append(int(np.random.normal(150, 20)))
    
    # Simulate PARBS latencies (high variance due to batching)
    latencies_parbs = []
    for _ in range(1000):
        if np.random.random() < 0.85:  # Higher hit rate
            latencies_parbs.append(int(np.random.normal(45, 15)))
        else:
            latencies_parbs.append(int(np.random.normal(300, 50)))  # High tail
    
    # Simulate BLP history
    blp_history = [np.random.poisson(6.5) for _ in range(2000)]
    
    # Per-thread latencies
    per_thread_latencies = {
        0: [int(np.random.normal(60, 15)) for _ in range(250)],
        1: [int(np.random.normal(55, 12)) for _ in range(250)],
        2: [int(np.random.normal(120, 30)) for _ in range(250)],
        3: [int(np.random.normal(110, 25)) for _ in range(250)]
    }
    
    # Row hit history
    row_hit_history = [np.random.random() < 0.78 for _ in range(1000)]
    
    # Analyze FR-FCFS
    stats_fr = analyzer.analyze_latency_distribution(latencies_fr_fcfs)
    stats_fr['blp'] = np.mean(blp_history)
    stats_fr['row_hit_rate'] = sum(row_hit_history) / len(row_hit_history)
    
    # Analyze PARBS
    stats_parbs = analyzer.analyze_latency_distribution(latencies_parbs)
    stats_parbs['blp'] = np.mean(blp_history) * 1.15  # PARBS gets better BLP
    stats_parbs['row_hit_rate'] = 0.85
    
    # Compare schedulers
    analyzer.compare_schedulers({
        'FR-FCFS': stats_fr,
        'PARBS': stats_parbs
    })
    
    # Generate comprehensive visualization
    analyzer.plot_comprehensive_analysis(
        latencies_fr_fcfs,
        blp_history,
        per_thread_latencies,
        row_hit_history,
        policy_name="FR-FCFS",
        output_file="../output/visualizations/fr_fcfs_analysis.png"
    )
    
    # Generate report
    analyzer.generate_report("FR-FCFS", stats_fr, 
                            "../output/reports/FR_FCFS_ANALYSIS.md")
    
    print("\n" + "="*80)
    print("✓ Analysis Complete!")
    print("="*80 + "\n")

if __name__ == "__main__":
    main()
