"""Compare outputs to those which are computed by the python bindings of FN5
"""
import pytest
import fn5

def test_1():
    '''This should be all comparisons
    '''
    with open("test/all.txt") as f:
        paths = [line.strip() for line in f]
    ref = fn5.load_reference("NC_045512.fasta")
    samples = [fn5.Sample(path, ref, set(), "sample"+path.split(".")[0][-1]) for path in paths]

    actual = set(fn5.compute(samples))
    actual_map = {}
    for a, b, dist in actual:
        actual_map[(a, b)] = dist
        actual_map[(b, a)] = dist
    expected = [
        "sample4 sample1 12",
        "sample4 sample3 11",
        "sample4 sample2 11",
        "sample1 sample3 1",
        "sample1 sample2 1",
        "sample3 sample2 0",
        ]
    expected = set([(x.split(" ")[0], x.split(" ")[1], int(x.split(" ")[2])) for x in expected])
    expected_map = {}
    for a, b, dist in expected:
        expected_map[(a, b)] = dist
        expected_map[(b, a)] = dist
    
    assert actual_map == expected_map