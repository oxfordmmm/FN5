'''Very simple querying of the output matrix to find nearest neighbours
'''

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--guid", required=True, help="GUID of the sample to query for")
    parser.add_argument("--snps", required=False, default=float('inf'), help="SNP threshold for nearest neighbour")
    options = parser.parse_args()

    guid = options.guid
    cutoff = float(options.snps)

    with open("outputs/all.txt") as f:
        data = [line.strip() for line in f]
    
    matches = {}
    for line in data:
        guid1, guid2, snp = line.split(" ")
        if guid1 == guid:
            snp = int(snp)
            if snp <= cutoff:
                matches[guid2] = snp
        elif guid2 == guid:
            snp = int(snp)
            if snp <=cutoff:
                matches[guid1] = snp
    
    print(matches)
