#!/usr/bin/env python3
"""
DRAM Timing Visualizer & Verification Tool - Lite Version
Optimized for large traces with sampling and reduced memory usage.
"""
import sys
import json
import csv
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

class LiteVisualizer:
    def __init__(self, trace_file, max_lines=5000):
        self.trace_file = trace_file
        self.max_lines = max_lines  # Limit processing for performance
        self.events = []
        self.power_profile = []
        self.max_cycle = 0
        self.sample_rate = 1  # Will be adjusted based on file size

        # Physical Constants (Datasheet IDD values in mA - DDR4 approx)
        self.IDD = {
            'ACT': 45.0,  'PRE': 20.0,
            'RD':  180.0, 'WR':  165.0,
            'REF': 240.0, 'IDLE': 10.0,
            'ACTIVATE': 45.0, 'PRECHARGE': 20.0,
            'READ': 180.0, 'WRITE': 165.0,
            'REFRESH': 240.0
        }

    def estimate_file_size(self):
        """Estimate total lines and adjust sampling rate"""
        try:
            with open(self.trace_file, 'r') as f:
                # Count first 1000 lines to estimate total
                line_count = 0
                for i, line in enumerate(f):
                    if i >= 1000:
                        break
                    line_count += 1
                
                # Estimate total lines
                f.seek(0, 2)  # Go to end
                file_size = f.tell()
                f.seek(0)     # Go back to start
                
                # Skip header to get average line length
                header = f.readline()
                pos_after_header = f.tell()
                avg_line_length = (file_size - pos_after_header) / max(line_count - 1, 1)
                estimated_lines = int(file_size / avg_line_length)
                
                print(f"Estimated {estimated_lines} total lines in trace file")
                
                # Adjust sample rate if file is large
                if estimated_lines > self.max_lines:
                    self.sample_rate = max(1, estimated_lines // self.max_lines)
                    print(f"Large file detected - sampling every {self.sample_rate} lines")
                
                return estimated_lines
        except:
            print("Could not estimate file size, proceeding with default sampling")
            return 0

    def parse_csv(self):
        """
        Parses CSV with intelligent sampling for large files
        """
        print(f"Loading trace: {self.trace_file} (lite processing)...")
        
        # Estimate file size and adjust sampling
        total_lines = self.estimate_file_size()
        
        # Streaming aggregation parameters
        bin_size = 1000  # Larger bins for performance
        power_bins = {}
        
        def cmd_to_state(cmd):
            up = cmd.upper()
            if up.startswith('ACT'):
                return 'ACTIVE'
            if up.startswith('READ') or up.startswith('RD'):
                return 'READING'
            if up.startswith('WRITE') or up.startswith('WR'):
                return 'WRITING'
            if up.startswith('PRE'):
                return 'PRECHARGING'
            if up.startswith('REF'):
                return 'REFRESHING'
            return None

        try:
            with open(self.trace_file, 'r') as f:
                reader = csv.DictReader(f)
                if not reader.fieldnames:
                    print('Warning: CSV appears empty.')
                    return

                current_state = 'IDLE'
                cur_start = 0
                cur_label = ''
                max_cycle = 0
                lines_processed = 0
                lines_sampled = 0

                for i, row in enumerate(reader):
                    lines_processed += 1
                    
                    # Sample based on sample_rate
                    if i % self.sample_rate != 0:
                        continue
                    
                    lines_sampled += 1
                    
                    # Stop if we've processed enough lines
                    if lines_sampled >= self.max_lines:
                        print(f"Reached sample limit of {self.max_lines} lines")
                        break

                    try:
                        cycle = int(row['Cycle'])
                        cmd = row['Command'].strip()
                        bank_row = row.get('Row', '0')
                    except (ValueError, KeyError):
                        continue

                    max_cycle = max(max_cycle, cycle)

                    # Bin power with larger bins
                    p = self.IDD.get(cmd, self.IDD.get(cmd[:3].upper(), 10.0))
                    b = cycle // bin_size
                    s = power_bins.get(b)
                    if s is None:
                        power_bins[b] = [p, 1]
                    else:
                        s[0] += p
                        s[1] += 1

                    # State transition logic
                    new_state = cmd_to_state(cmd)
                    if new_state is None:
                        continue

                    if new_state != current_state:
                        if cycle > cur_start:
                            self.events.append({
                                'State': current_state, 
                                'Start': cur_start, 
                                'End': cycle, 
                                'Label': cur_label
                            })
                        current_state = new_state
                        cur_start = cycle
                        cur_label = (f'Row {bank_row}' if new_state == 'ACTIVE' else '')
                    else:
                        if new_state == 'ACTIVE':
                            cur_label = f'Row {bank_row}'

                # Finalize last interval
                if max_cycle >= cur_start:
                    self.events.append({
                        'State': current_state, 
                        'Start': cur_start, 
                        'End': max_cycle + 1, 
                        'Label': cur_label
                    })

                # Convert power_bins to profile
                bins = sorted(power_bins.items())
                self.power_profile = []
                for bin_idx, (sum_ma, cnt) in bins:
                    start = bin_idx * bin_size
                    avg = sum_ma / cnt if cnt else 0.0
                    self.power_profile.append((start, avg))

                self.max_cycle = max_cycle

                print(f'Processed {lines_processed} total lines, sampled {lines_sampled} lines')
                print(f'Generated {len(self.events)} state intervals and {len(self.power_profile)} power bins')

                # Write summary files
                with open('summary_trace_lite.csv', 'w', newline='') as out:
                    w = csv.writer(out)
                    w.writerow(['Start', 'End', 'State', 'Label'])
                    for e in self.events:
                        w.writerow([e['Start'], e['End'], e['State'], e['Label']])

                with open('power_bins_lite.csv', 'w', newline='') as out:
                    w = csv.writer(out)
                    w.writerow(['BinStart', 'BinEnd', 'AvgCurrent_mA'])
                    for start, avg in self.power_profile:
                        w.writerow([start, start + bin_size, avg])

        except FileNotFoundError:
            print(f"Error: Trace file '{self.trace_file}' not found.")
            sys.exit(1)

    def plot(self):
        if not self.events:
            print("No events found to plot.")
            return

        # Limit events for visualization but focus on a meaningful time window
        max_events = 50  # Fewer events for MUCH clearer visualization
        
        # Find first period of activity for better view
        active_events = [e for e in self.events if e['State'] != 'IDLE']
        if active_events and len(active_events) > max_events:
            print(f"Found {len(active_events)} active events, showing first {max_events} for clarity")
            events_to_plot = active_events[:max_events]
        elif active_events:
            events_to_plot = active_events
        else:
            events_to_plot = self.events[:max_events] if len(self.events) > max_events else self.events

        # TWO SEPARATE FIGURES - one for bank state, one for power
        # This avoids the x-axis sharing problem!
        
        # ============== FIGURE 1: Bank State Timeline ==============
        fig1, ax1 = plt.subplots(figsize=(22, 8))
        
        # Vibrant colors
        colors = {
            'IDLE': '#e0e0e0',      'ACTIVE': '#4caf50',
            'READING': '#2196f3',   'WRITING': '#f44336',
            'REFRESHING': '#9c27b0','PRECHARGING': '#ff9800'
        }

        # Get the actual time range of our events
        start_cycle = events_to_plot[0]['Start']
        end_cycle = events_to_plot[-1]['End']
        time_span = end_cycle - start_cycle
        
        print(f"Plotting {len(events_to_plot)} events from cycle {start_cycle} to {end_cycle}")
        
        # Draw each state as a full-height bar
        for e in events_to_plot:
            c = colors.get(e['State'], '#cfd8dc')
            duration = e['End'] - e['Start']
            
            # FULL HEIGHT bars from 0 to 1
            ax1.barh(y=0.5, width=duration, left=e['Start'], height=0.8,
                    color=c, edgecolor='black', linewidth=1.5, alpha=0.95)
            
            # Add labels on bars that are wide enough
            bar_width_ratio = duration / time_span
            if bar_width_ratio > 0.02:  # Only label if bar is > 2% of total width
                text_x = e['Start'] + duration/2
                
                # Show state abbreviation or row label
                if e['Label']:
                    display_text = e['Label']
                else:
                    display_text = e['State'][:4]
                
                ax1.text(text_x, 0.5, display_text, 
                        ha='center', va='center', fontsize=11, 
                        color='white', fontweight='bold',
                        bbox=dict(boxstyle="round,pad=0.2", facecolor='black', alpha=0.7))

        # Set x-axis to EXACTLY fit the events (no wasted space!)
        ax1.set_xlim(start_cycle - time_span*0.02, end_cycle + time_span*0.02)
        ax1.set_ylim(0, 1)
        ax1.set_yticks([])
        
        # Enhanced legend
        patches = [mpatches.Patch(color=v, label=k) for k, v in colors.items()]
        ax1.legend(handles=patches, loc='upper right', ncol=6, 
                  fontsize=12, frameon=True, shadow=True)
        
        ax1.set_xlabel("Simulation Cycles", fontsize=14, fontweight='bold')
        ax1.set_ylabel("Bank State", fontsize=14, fontweight='bold')
        ax1.set_title(f"DRAM Bank State Timeline (Cycles {start_cycle:,} to {end_cycle:,})", 
                     fontsize=16, fontweight='bold', pad=15)
        ax1.grid(True, axis='x', linestyle=':', alpha=0.5)
        
        plt.tight_layout()
        plt.savefig("dram_bank_states.png", dpi=150, bbox_inches='tight', facecolor='white')
        print("Bank state timeline saved to dram_bank_states.png")
        
        # ============== FIGURE 2: Power Profile ==============
        fig2, ax2 = plt.subplots(figsize=(18, 6))
        
        if self.power_profile:
            x_vals, y_vals = zip(*self.power_profile)
            ax2.step(x_vals, y_vals, where='post', color='#d32f2f', linewidth=2)
            ax2.fill_between(x_vals, y_vals, step='post', alpha=0.25, color='#d32f2f')
            
            ax2.axhline(y=50, color='green', linestyle='--', alpha=0.6, linewidth=1.5, label='Low Power')
            ax2.axhline(y=150, color='orange', linestyle='--', alpha=0.6, linewidth=1.5, label='High Power')

        ax2.set_ylabel("IDD Current (mA)", fontsize=14, fontweight='bold')
        ax2.set_xlabel("Simulation Cycles", fontsize=14, fontweight='bold')
        ax2.set_ylim(0, 280)
        ax2.grid(True, linestyle='--', alpha=0.5)
        ax2.set_title("Power Consumption Profile", fontsize=16, fontweight='bold')
        ax2.legend(loc='upper right', fontsize=11)
        
        plt.tight_layout()
        plt.savefig("dram_power_profile.png", dpi=150, bbox_inches='tight', facecolor='white')
        print("Power profile saved to dram_power_profile.png")
        
        plt.show()

    def print_summary(self):
        """Print a quick summary of the analysis"""
        if not self.events:
            print("No events to summarize.")
            return
            
        print("\n" + "="*50)
        print("DRAM TIMING ANALYSIS SUMMARY")
        print("="*50)
        
        # State distribution
        state_counts = {}
        total_cycles = 0
        for e in self.events:
            duration = e['End'] - e['Start']
            state_counts[e['State']] = state_counts.get(e['State'], 0) + duration
            total_cycles += duration
        
        print(f"Total simulation cycles: {self.max_cycle}")
        print(f"State distribution:")
        for state, cycles in sorted(state_counts.items()):
            percentage = (cycles / total_cycles * 100) if total_cycles > 0 else 0
            print(f"  {state:12}: {cycles:8} cycles ({percentage:5.1f}%)")
        
        # Power analysis
        if self.power_profile:
            avg_power = sum(power for _, power in self.power_profile) / len(self.power_profile)
            max_power = max(power for _, power in self.power_profile)
            print(f"\nPower analysis:")
            print(f"  Average current: {avg_power:6.1f} mA")
            print(f"  Peak current:    {max_power:6.1f} mA")

if __name__ == "__main__":
    infile = sys.argv[1] if len(sys.argv) > 1 else "trace_dramsim3.txt"
    max_lines = int(sys.argv[2]) if len(sys.argv) > 2 else 5000
    
    print(f"DRAM Lite Visualizer - Processing max {max_lines} lines")
    
    viz = LiteVisualizer(infile, max_lines)
    viz.parse_csv()
    viz.print_summary()
    viz.plot()