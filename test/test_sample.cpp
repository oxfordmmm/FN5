#include <gtest/gtest.h>
#include <unordered_set>
#include "../src/include/sample.hpp"

TEST(sample, init){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    string expected_reference = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    EXPECT_EQ(expected_reference, reference);

    unordered_set<int> expected_mask;
    expected_mask.insert(1);
    EXPECT_EQ(expected_mask.size(), mask.size());

}

