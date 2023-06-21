'''Just wait until this thread's lock is the next lock
'''
import argparse
from model import *


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lock", required=True, type=int, help="Thread ID for this lock")
    options = parser.parse_args()

    conn, engine = get_engine()

    get_lock_(engine, options.lock)
