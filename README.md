# Kernel Feature Implementation
A collection of three Linux Kernel Modules demonstrating process scheduling, inter-process communication, and process migration capabilities.
## Overview
This project implements three kernel-level features:

- Priority-Based Scheduler: Dynamic process priority management through /proc interface
- Priority Message Queue: Advanced IPC with priority-based message handling
- Process Migration: Simulates process migration across CPU cores

## Prerequisites
```bash
sudo apt update

sudo apt install -y build-essential linux-headers-$(uname -r) git
```
## Quick Start
### 1. Priority Scheduler
```bash
cd priority_scheduler
make
sudo insmod priority_scheduler.ko

# Set priority for a process (0=highest, 10=lowest)
echo "1234 5" | sudo tee /proc/priority_sched

sudo rmmod priority_scheduler
```
### 2. Priority Message Queue
```bash
cd safe_lkm
make
sudo insmod safe_lkm.ko

# Send a message: B <pid> <type> <message>
echo "B 1001 6 Hello" | sudo tee /proc/safe_lkm

# Receive message
echo "R" | sudo tee /proc/safe_lkm

sudo rmmod safe_lkm
```
### 3. Process Migration
```bash
cd process_migration
make

# Migrate specific processes (comma-separated PIDs)
sudo insmod migration_module.ko target_pids="1234,5678"

sudo rmmod migration_module
```
## Viewing Logs
```bash
sudo dmesg | tail -20
```

## Project Structure
```
├── priority_scheduler/    # Dynamic priority assignment
├── safe_lkm/             # Priority-based message queue
└── process_migration/    # Process migration simulator
```
## Features
- Thread-safe operations using kernel synchronisation primitives
- Dynamic priority mapping to Linux nice values
- Priority message handling (high-priority processed first)
- Robust error handling for invalid inputs

## Notes

- Run all commands with sudo
- Test in a VM environment
- Check dmesg for module output
- Unload modules before reloading

## Team
Laiba Riaz • Zaynab Shahid • Rameen Arshad • Bilal Rana
