'''Release the lock from the DB
'''
import argparse
from model import *


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lock", required=True, type=int, help="Thread ID for this lock")
    options = parser.parse_args()

    conn, engine = get_engine()

    unlock_(engine, options.lock)
