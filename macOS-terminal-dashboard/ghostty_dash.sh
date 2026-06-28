#!/usr/bin/env bash
#
# StackTrace — v1 shell prototype
# Terminal workspace dashboard: orchestrates 8 CLI tools into a tiled
# tmux layout inside a single Ghostty window.
#
#   Row 1:  fastfetch  |  tty-clock  |  ticker
#   Row 2:  btop       |  calcurse   |  yazi
#   Row 3:  newsboat   |  nvim
#
# Usage: ./ghostty_dash.sh

set -euo pipefail

SESSION="stacktrace"

# Kill any existing session with the same name (ignore error if absent).
tmux kill-session -t "$SESSION" 2>/dev/null || true

# Start a new detached session with a single window named 'dash'.
tmux new-session -d -s "$SESSION" -n dash

# ───────────── Top Row ─────────────
# Pane 0: fastfetch (default pane)
# Pane 1: split right of 0 → tty-clock
tmux split-window -h -t "$SESSION:0.0"
# Pane 2: split right of 1 → ticker
tmux split-window -h -t "$SESSION:0.1"

# ───────────── Middle Row ─────────────
# Pane 3: split below pane 0 → btop
tmux split-window -v -t "$SESSION:0.0"
# Pane 4: split below pane 1 → calcurse
tmux split-window -v -t "$SESSION:0.1"
# Pane 5: split below pane 2 → yazi
tmux split-window -v -t "$SESSION:0.2"

# ───────────── Bottom Row ─────────────
# Pane 6: split below pane 4 → newsboat
tmux split-window -v -t "$SESSION:0.4"
# Pane 7: split right of pane 6 → nvim notes
tmux split-window -h -t "$SESSION:0.6"

# ───────────── Launch each tool in its pane ─────────────
tmux send-keys -t "$SESSION:0.0" 'fastfetch' C-m
tmux send-keys -t "$SESSION:0.1" 'tty-clock -c -C 5 -s -B' C-m
tmux send-keys -t "$SESSION:0.2" 'ticker --watchlist AAPL,NVDA,TSLA,MSFT,GOOG,SPY --show-summary --show-tags --show-fundamentals --show-separator --sort alpha' C-m
tmux send-keys -t "$SESSION:0.3" 'btop' C-m
tmux send-keys -t "$SESSION:0.4" 'calcurse' C-m
tmux send-keys -t "$SESSION:0.5" 'yazi ~/Documents' C-m
tmux send-keys -t "$SESSION:0.6" 'newsboat' C-m
tmux send-keys -t "$SESSION:0.7" 'nvim ~/notes.txt' C-m

# Normalize the arrangement into an even tiled grid.
tmux select-layout -t "$SESSION" tiled

# Focus the top-left pane before attaching.
tmux select-pane -t "$SESSION:0.0"

# Attach.
tmux attach-session -t "$SESSION"
