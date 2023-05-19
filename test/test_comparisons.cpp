#include <fstream>
#include <gtest/gtest.h>
#include "../src/include/comparisons.hpp"

/**
* @brief Check if two vectors contain the same elements (not caring about order)
*/
bool vectors_equal(vector<Sample*> v1, vector<Sample*> v2){
    if(v1.size() != v2.size()){
        return false;
    }
    bool all_match = true;
    for(int i=0;i<v1.size();i++){
        bool elem_match = false;
        for(int j=0;j<v2.size();j++){
            if(*v1.at(i) == *v2.at(j)){
                elem_match = true;
            }
        }
        all_match = all_match && elem_match;
    }
    return all_match;
}

/**
* @brief Test `load_saves`
*/
TEST(comparisons, load_saves){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    save("cases/dummy/saves", s1);
    save("cases/dummy/saves", s2);
    save("cases/dummy/saves", s3);
    save("cases/dummy/saves", s4);
    save("cases/dummy/saves", s5);

    save_dir = "cases/dummy/saves";
    vector<Sample*> actual = load_saves();
    vector<Sample*> expected = {s1, s2, s3, s4, s5};

    ASSERT_TRUE(vectors_equal(expected, actual));
}

/**
* @brief Test `load_save_thread`
*/
TEST(comparisons, load_save_thread){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    save("cases/dummy/saves", s1);
    save("cases/dummy/saves", s2);
    save("cases/dummy/saves", s3);
    save("cases/dummy/saves", s4);
    save("cases/dummy/saves", s5);

    vector<Sample*> expected = {s1, s2, s3, s4, s5};


    vector<Sample*> acc;
    vector<string> filenames = {"cases/dummy/saves/uuid1", "cases/dummy/saves/uuid2", "cases/dummy/saves/uuid3", "cases/dummy/saves/uuid4", "cases/dummy/saves/uuid5"};
    load_save_thread(filenames, &acc);

    ASSERT_TRUE(vectors_equal(expected, acc));

}

/**
* @brief Test `load_saves_multithreaded`
*/
TEST(comparions, load_saves_multithreaded){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    save("cases/dummy/saves", s1);
    save("cases/dummy/saves", s2);
    save("cases/dummy/saves", s3);
    save("cases/dummy/saves", s4);
    save("cases/dummy/saves", s5);

    vector<Sample*> expected = {s1, s2, s3, s4, s5};
    save_dir = "cases/dummy/saves";

    vector<Sample*> actual = load_saves_multithreaded();
    ASSERT_TRUE(vectors_equal(expected, actual));
}

/**
* @brief Test `parse_n`
*/
TEST(comparisons, parse_n){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    vector<Sample*> expected = {s1, s2, s3, s4, s5};

    vector<string> filenames = {"cases/dummy/1.fasta", "cases/dummy/2.fasta", "cases/dummy/3.fasta", "cases/dummy/4.fasta", "cases/dummy/5.fasta"};
    vector<Sample*> acc;

    parse_n(filenames, reference, mask, &acc);

    ASSERT_TRUE(vectors_equal(expected, acc));

}

/**
* @brief Test `bulk_load`
*/
TEST(comparisons, bulk_load){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    //Deliberately not using sample 4, to ensure the list reading works
    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    vector<Sample*> expected = {s1, s2, s3, s5};

    vector<string> filenames = {"cases/dummy/1.fasta", "cases/dummy/2.fasta", "cases/dummy/3.fasta", "cases/dummy/5.fasta"};
    fstream out("dummy_samples.txt", fstream::out);
    for(string elem : filenames){
        out << elem << endl;
    }
    out.close();

    vector<Sample*> actual = bulk_load("dummy_samples.txt", reference, mask);
    ASSERT_TRUE(vectors_equal(expected, actual));
}

/**
* @brief Test `save_comparisons`
*/
TEST(comparisons, save_comparisons){
    output_file = "test_save_comparisons.txt";
    //Random dummy data here
    vector<tuple<string, string, int>> comparisons = {
        {"guid1", "guid2", 0},
        {"guid1", "guid3", 1},
        {"guid1", "guid4", 2},
        {"guid1", "guid5", 0},
        {"guid1", "guid6", 9},
    };
    string expected = "guid1 guid2 0\nguid1 guid3 1\nguid1 guid4 2\nguid1 guid5 0\nguid1 guid6 9\n";

    //Ensure output file is empty first
    fstream out(output_file, fstream::out);
    out.close();
    save_comparisons(comparisons);

    fstream in("test_save_comparisons.txt", fstream::in);
    string actual;
    char c;
    while (in >> noskipws >> c){
        actual += c;
    }
    in.close();

    ASSERT_EQ(expected, actual);

}

/**
* @brief Test `print_comparisons`
*/
TEST(comparisons, print_comparisons){
    //Random dummy data here
    vector<tuple<string, string, int>> comparisons = {
        {"guid1", "guid2", 0},
        {"guid1", "guid3", 1},
        {"guid1", "guid4", 2},
        {"guid1", "guid5", 0},
        {"guid1", "guid6", 9},
    };
    string expected = "guid1 guid2 0\nguid1 guid3 1\nguid1 guid4 2\nguid1 guid5 0\nguid1 guid6 9\n";

    //Use GTest to get the stdout
    testing::internal::CaptureStdout();
    print_comparisons(comparisons);
    string actual = testing::internal::GetCapturedStdout();

    ASSERT_EQ(expected, actual);

}



