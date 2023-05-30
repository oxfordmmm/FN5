import pytest

'''All files are parsed into a set to avoid order based issues due to multithreading
'''

def test_1():
    '''This should be all comparisons
    '''
    with open("test/output/1.txt") as f:
        actual = set([tuple(sorted(line.strip().split(" "))) for line in f])
    
    expected = [
        "sample4 sample1 12",
        "sample4 sample3 11",
        "sample4 sample2 11",
        "sample1 sample3 1",
        "sample1 sample2 1",
        "sample3 sample2 0",
        ]
    expected = set([tuple(sorted(x.split(" "))) for x in expected])
    
    assert actual == expected

def test_2():
    '''This should be adding a single sample
    '''
    with open("test/output/2.txt") as f:
        actual = set([tuple(sorted(line.strip().split(" "))) for line in f])
    
    expected = set([
        "sample4 sample1 12",
        "sample4 sample3 11",
        "sample4 sample2 11",
        ])
    expected = set([tuple(sorted(x.split(" "))) for x in expected])

    assert actual == expected

def test_3():
    '''This should be adding samples 1 and 2
    '''
    with open("test/output/3.txt") as f:
        actual = set([tuple(sorted(line.strip().split(" "))) for line in f])
    
    expected = set([
        "sample1 sample4 12",
        "sample3 sample1 1",
        "sample3 sample4 11",
        "sample2 sample1 1",
        "sample2 sample4 11",
        ])
    expected = set([tuple(sorted(x.split(" "))) for x in expected])

    assert actual == expected

def test_4():
    '''This should just be adding sample4
    '''
    with open("test/output/4.txt") as f:
        actual = set([tuple(sorted(line.strip().split(" "))) for line in f])

    expected = [
            "sample4 sample1 12",
            "sample4 sample3 11",
            "sample4 sample2 11",
        ]
    expected = set([tuple(sorted(x.split(" "))) for x in expected])
    assert actual == expected