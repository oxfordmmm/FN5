#include <fstream>
#include <gtest/gtest.h>
#include "../src/include/comparisons.hpp"

namespace fs = std::filesystem;


/**
* @brief Check if two vectors contain the same elements (not caring about order)
*/
bool vectors_equal(vector<Sample*> v1, vector<Sample*> v2){
    if(v1.size() != v2.size()){
        cout << "Sizes don't match: " << v1.size() << " != " << v2.size() << endl;
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
* @brief Overloaded for strings
*/
bool vectors_equal(vector<string> v1, vector<string> v2){
    if(v1.size() != v2.size()){
        cout << "Sizes don't match: " << v1.size() << " != " << v2.size() << endl;
        return false;
    }
    bool all_match = true;
    for(int i=0;i<v1.size();i++){
        bool elem_match = false;
        for(int j=0;j<v2.size();j++){
            if(v1.at(i) == v2.at(j)){
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
    vector<string> filenames = {"cases/dummy/saves/uuid1.fn5", "cases/dummy/saves/uuid2.fn5", "cases/dummy/saves/uuid3.fn5", "cases/dummy/saves/uuid4.fn5", "cases/dummy/saves/uuid5.fn5"};
    load_save_thread(filenames, &acc);

    ASSERT_TRUE(vectors_equal(expected, acc));

}

/**
* @brief Test `load_saves_multithreaded`
*/
TEST(comparisons, load_saves_multithreaded){
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

/**
* @brief Test `save_comparisons`
*/
TEST(comparisons, do_comparisons_from_disk){
    output_file = "test_do_comparisons_from_disk.txt";
    
    vector<string> paths = {"cases/dummy/saves/uuid1.fn5", "cases/dummy/saves/uuid2.fn5", "cases/dummy/saves/uuid3.fn5", "cases/dummy/saves/uuid4.fn5"};
    Sample *s = readSample("cases/dummy/saves/uuid5.fn5");

    //Ensure output file is empty first
    fstream out(output_file, fstream::out);
    out.close();

    //Do the comparisons
    do_comparisons_from_disk(paths, s, 99999);

    fstream in(output_file, fstream::in);
    string actual;
    char c;
    while (in >> noskipws >> c){
        actual += c;
    }
    in.close();

    //Expected data
    string expected = "uuid5 uuid1 79\nuuid5 uuid2 79\nuuid5 uuid3 77\nuuid5 uuid4 78\n";
    ASSERT_EQ(expected, actual);

}

/**
* @brief Test `add_sample`
*/
TEST(comparisons, add_sample){
    //'add' sample 5 to the existing saves (1-5)
    save_dir = "cases/dummy/saves";
    output_file = "test_add_sample.txt";

    //Ensure output file is empty first
    fstream out(output_file, fstream::out);
    out.close();

    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    add_sample("cases/dummy/5.fasta", reference, mask, 99999);

    fstream in(output_file, fstream::in);
    vector<string> actual;
    string acc;
    char c;
    while (in >> noskipws >> c){
        if(c == '\n'){
            actual.push_back(acc);
            acc = "";
        }
        else{
            acc += c;
        }
    }
    in.close();

    //Expected data
    vector<string> expected = {"uuid5 uuid1 79", "uuid5 uuid2 79", "uuid5 uuid3 77", "uuid5 uuid4 78"};
    ASSERT_TRUE(vectors_equal(expected, actual));
}


/**
* @brief Test `do_comparisons`
*/
TEST(comparisons, do_comparisons){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);

    vector<tuple<Sample*, Sample*>> to_compare = {
        {s1, s2}, {s1, s3}, {s2, s3}
    };

    //Use GTest to get the stdout
    testing::internal::CaptureStdout();
    do_comparisons(to_compare, 99999);
    string actual = testing::internal::GetCapturedStdout();
    string expected = "uuid1 uuid2 1\nuuid1 uuid3 1\nuuid2 uuid3 1\n";
    ASSERT_EQ(expected, actual);
}

/**
* @brief Test `add_many`
*/
TEST(comparisons, add_many){
    //Clear the saves dir first
    save_dir = "cases/dummy/saves";
    for (const auto & entry : fs::directory_iterator(save_dir)){
        string p = entry.path();
        remove(p.c_str());
    }

    //We're adding sample 4 and 5 for the test here
    vector<string> filenames = {"cases/dummy/4.fasta", "cases/dummy/5.fasta"};
    fstream out("dummy_samples2.txt", fstream::out);
    for(string elem : filenames){
        out << elem << endl;
    }
    out.close();

    //Save sample 1, 2, and 3 to add to
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);

    save("cases/dummy/saves", s1);
    save("cases/dummy/saves", s2);
    save("cases/dummy/saves", s3);

    //Use GTest to get the stdout
    testing::internal::CaptureStdout();
    add_many("dummy_samples2.txt", reference, mask, 99999);
    string actual = testing::internal::GetCapturedStdout();

    //Order isn't important here, so split into vector for comparison
    vector<string> actual_split;
    string acc;
    for(int i=0;i<actual.size();i++){
        if(actual.at(i) == '\n'){
            actual_split.push_back(acc);
            acc = "";
        }
        else{
            acc += actual.at(i);
        }
    }

    //Expected comparisons
    vector<string> expected = {
        "uuid1 uuid4 1",
        "uuid2 uuid4 1",
        "uuid3 uuid4 2",
        "uuid1 uuid5 79",
        "uuid2 uuid5 79",
        "uuid3 uuid5 77",
        "uuid4 uuid5 78"
    };

    ASSERT_TRUE(vectors_equal(expected, actual_split));

    //Check that the added samples are saved too
    vector<Sample*> actual_samples = load_saves();
    vector<Sample*> expected_samples = {
        s1, s2, s3, new Sample("cases/dummy/4.fasta", reference, mask), new Sample("cases/dummy/5.fasta", reference, mask)
    };

    ASSERT_TRUE(vectors_equal(expected_samples, actual_samples));

}


/**
* @brief Test `compare_row`
*/
TEST(comparisons, compare_row){
    //'add' sample 5 to the existing saves (1-5)
    save_dir = "cases/dummy/saves";

    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    //Use GTest to get the stdout
    testing::internal::CaptureStdout();
    compare_row("cases/dummy/5.fasta", reference, mask, 99999);
    string actual = testing::internal::GetCapturedStdout();

    //Order isn't important here, so split into vector for comparison
    vector<string> actual_split;
    string acc;
    for(int i=0;i<actual.size();i++){
        if(actual.at(i) == '\n'){
            actual_split.push_back(acc);
            acc = "";
        }
        else{
            acc += actual.at(i);
        }
    }

    //Expected data
    vector<string> expected = {"uuid5 uuid1 79", "uuid5 uuid2 79", "uuid5 uuid3 77", "uuid5 uuid4 78"};
    ASSERT_TRUE(vectors_equal(expected, actual_split));
}

/**
* @brief Test `compute_loaded`
*/
TEST(comparisons, compute_loaded){
    //Utilise previously tested items (do_comparisons) to test this
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    vector<Sample*> samples = {s1, s2, s3, s4, s5};
    vector<tuple<Sample*, Sample*>> to_compare = {
        {s1, s2}, {s1, s3}, {s1, s4}, {s1, s5},
        {s2, s3}, {s2, s4}, {s2, s5},
        {s3, s4}, {s3, s5},
        {s4, s5}
    };

    //Use GTest to get the stdout
    testing::internal::CaptureStdout();
    //Use do_comparisons to populate expected outputs
    do_comparisons(to_compare, 999999);
    string expected = testing::internal::GetCapturedStdout();

    //Order isn't important here, so split into vector for comparison
    vector<string> expected_split;
    string acc;
    for(int i=0;i<expected.size();i++){
        if(expected.at(i) == '\n'){
            expected_split.push_back(acc);
            acc = "";
        }
        else{
            acc += expected.at(i);
        }
    }

    testing::internal::CaptureStdout();
    compute_loaded(999999, samples);
    string actual = testing::internal::GetCapturedStdout();

    //Order isn't important here, so split into vector for comparison
    vector<string> actual_split;
    acc = "";
    for(int i=0;i<actual.size();i++){
        if(actual.at(i) == '\n'){
            actual_split.push_back(acc);
            acc = "";
        }
        else{
            acc += actual.at(i);
        }
    }

    ASSERT_TRUE(vectors_equal(expected_split, actual_split));
}


