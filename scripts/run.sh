#!/bin/bash
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
    source .venv/bin/activate
    pip install -r requirements-cli.txt
else
    source .venv/bin/activate
fi
python3 icha_cli.py "$@"
