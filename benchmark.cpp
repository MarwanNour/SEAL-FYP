#include <iostream>
#include <fstream>
#include <iomanip>
#include "seal/seal.h"

using namespace std;
using namespace seal;

// Helper function that prints a vector of floats
template <typename T>
inline void print_vector(std::vector<T> vec, std::size_t print_size = 4, int prec = 3)
{
    /*
    Save the formatting information for std::cout.
    */
    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    std::size_t slot_count = vec.size();

    std::cout << std::fixed << std::setprecision(prec);
    std::cout << std::endl;
    if (slot_count <= 2 * print_size)
    {
        std::cout << "    [";
        for (std::size_t i = 0; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    else
    {
        vec.resize(std::max(vec.size(), 2 * print_size));
        std::cout << "    [";
        for (std::size_t i = 0; i < print_size; i++)
        {
            std::cout << " " << vec[i] << ",";
        }
        if (vec.size() > 2 * print_size)
        {
            std::cout << " ...,";
        }
        for (std::size_t i = slot_count - print_size; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    std::cout << std::endl;

    /*
    Restore the old std::cout formatting.
    */
    std::cout.copyfmt(old_fmt);
}

void ckksBenchmark(size_t poly_modulus_degree)
{
    cout << "------CKKS TEST------\n"
         << endl;

    // Set params
    EncryptionParameters params(scheme_type::CKKS);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    auto context = SEALContext::Create(params);

    // Generate keys, encryptor, decryptor and evaluator
    KeyGenerator keygen(context);
    PublicKey pk = keygen.public_key();
    SecretKey sk = keygen.secret_key();

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);

    // Create CKKS encoder
    CKKSEncoder ckks_encoder(context);

    size_t slot_count = ckks_encoder.slot_count();
    cout << "Slot count : " << slot_count << endl;

    /*
    3 sets of vectors:
    1st set: sizes = 10
    2nd set: sizes = 100
    3rd set: sizes = 1000
    */

    // ------------- FIRST SET -------------
    // First vector
    vector<double> pod_vec1_set1(10, 0);
    for (unsigned int i = 0; i < 10; i++)
    {
        pod_vec1_set1[i] = static_cast<double>(i);
    }
    print_vector(pod_vec1_set1);
    // Second vector
    vector<double> pod_vec2_set1(10, 0);
    for (unsigned int i = 0; i < 10; i++)
    {
        pod_vec2_set1[i] = static_cast<double>((i % 2) + 1);
    }
    print_vector(pod_vec2_set1);

    // -------------- SECOND SET -------------
    // First vector
    vector<double> pod_vec1_set2(100, 0);
    for (unsigned int i = 0; i < 100; i++)
    {
        pod_vec1_set2[i] = static_cast<double>(i);
    }
    print_vector(pod_vec1_set2);
    // Second vector
    vector<double> pod_vec2_set2(100, 0);
    for (unsigned int i = 0; i < 100; i++)
    {
        pod_vec2_set2[i] = static_cast<double>((i % 2) + 1);
    }
    print_vector(pod_vec2_set2);

    // -------------- THIRD SET -------------
    // First vector
    vector<double> pod_vec1_set3(1000, 0);
    for (unsigned int i = 0; i < 1000; i++)
    {
        pod_vec1_set3[i] = static_cast<double>(i);
    }
    print_vector(pod_vec1_set3);
    // Second vector
    vector<double> pod_vec2_set3(1000, 0);
    for (unsigned int i = 0; i < 1000; i++)
    {
        pod_vec2_set3[i] = static_cast<double>((i % 2) + 1);
    }
    print_vector(pod_vec2_set3);

    // Encode all vectors
    Plaintext plain_vec1_set1, plain_vec2_set1, plain_vec1_set2, plain_vec2_set2, plain_vec1_set3, plain_vec2_set3;
    double scale = sqrt(static_cast<double>(params.coeff_modulus().back().value()));
    // First set encode
    ckks_encoder.encode(pod_vec1_set1, scale, plain_vec1_set1);
    ckks_encoder.encode(pod_vec2_set1, scale, plain_vec2_set1);
    // Second set encode
    ckks_encoder.encode(pod_vec1_set2, scale, plain_vec1_set2);
    ckks_encoder.encode(pod_vec2_set2, scale, plain_vec2_set2);
    // Third set encode
    ckks_encoder.encode(pod_vec1_set3, scale, plain_vec1_set3);
    ckks_encoder.encode(pod_vec2_set3, scale, plain_vec2_set3);

    // Encrypt all vectors
    Ciphertext cipher_vec1_set1, cipher_vec2_set1, cipher_vec1_set2, cipher_vec2_set2, cipher_vec1_set3, cipher_vec2_set3;
    // First set cipher
    encryptor.encrypt(plain_vec1_set1, cipher_vec1_set1);
    encryptor.encrypt(plain_vec2_set1, cipher_vec2_set1);
    // Second set cipher
    encryptor.encrypt(plain_vec1_set2, cipher_vec1_set2);
    encryptor.encrypt(plain_vec2_set2, cipher_vec2_set2);
    // Third set cipher
    encryptor.encrypt(plain_vec1_set3, cipher_vec1_set3);
    encryptor.encrypt(plain_vec2_set3, cipher_vec2_set3);
    // Create Ciphertext Outputs
    Ciphertext cipher_result1_set1, cipher_result1_set2, cipher_result1_set3;
    Ciphertext cipher_result2_set1, cipher_result2_set2, cipher_result2_set3;
    Ciphertext cipher_result3_set1, cipher_result3_set2, cipher_result3_set3;
    Ciphertext cipher_result4_set1, cipher_result4_set2, cipher_result4_set3;

    // ------------------ (cipher1 + plain2) ---------------
    cout << "\n------------------ FIRST OPERATION ------------------\n"
         << endl;
    // Compute (cipher1 + plain2) for set 1
    cout << "Compute (cipher1 + plain2) for set 1" << endl;

    // TIME START
    auto start_comp1_set1 = chrono::high_resolution_clock::now();

    evaluator.add_plain(cipher_vec1_set1, plain_vec2_set1, cipher_result1_set1);

    // TIME END
    auto stop_comp1_set1 = chrono::high_resolution_clock::now();
    auto duration_comp1_set1 = chrono::duration_cast<chrono::microseconds>(stop_comp1_set1 - start_comp1_set1);

    // Decrypt and Decode
    Plaintext plain_result1_set1;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result1_set1, plain_result1_set1);
    vector<double> vec_result1_set1;
    ckks_encoder.decode(plain_result1_set1, vec_result1_set1);
    print_vector(vec_result1_set1);

    cout << "\nTime to compute (cipher1 + plain2): " << duration_comp1_set1.count() << " microseconds" << endl;

    // Compute (cipher1 + plain2) for set 2
    cout << "Compute (cipher1 + plain2) for set 2" << endl;

    // TIME START
    auto start_comp1_set2 = chrono::high_resolution_clock::now();

    evaluator.add_plain(cipher_vec1_set2, plain_vec2_set2, cipher_result1_set2);

    // TIME END
    auto stop_comp1_set2 = chrono::high_resolution_clock::now();
    auto duration_comp1_set2 = chrono::duration_cast<chrono::microseconds>(stop_comp1_set2 - start_comp1_set2);

    // Decrypt and Decode
    Plaintext plain_result1_set2;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result1_set2, plain_result1_set2);
    vector<double> vec_result1_set2;
    ckks_encoder.decode(plain_result1_set2, vec_result1_set2);
    print_vector(vec_result1_set2);

    cout << "\nTime to compute (cipher1 + plain2): " << duration_comp1_set2.count() << " microseconds" << endl;

    // Compute (cipher1 + plain2) for set 3
    cout << "Compute (cipher1 + plain2) for set 3" << endl;

    // TIME START
    auto start_comp1_set3 = chrono::high_resolution_clock::now();

    evaluator.add_plain(cipher_vec1_set3, plain_vec2_set3, cipher_result1_set3);

    // TIME END
    auto stop_comp1_set3 = chrono::high_resolution_clock::now();
    auto duration_comp1_set3 = chrono::duration_cast<chrono::microseconds>(stop_comp1_set3 - start_comp1_set3);

    // Decrypt and Decode
    Plaintext plain_result1_set3;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result1_set3, plain_result1_set3);
    vector<double> vec_result1_set3;
    ckks_encoder.decode(plain_result1_set3, vec_result1_set3);
    print_vector(vec_result1_set3);

    cout << "\nTime to compute (cipher1 + plain2): " << duration_comp1_set3.count() << " microseconds" << endl;

    cout << endl;

    // ------------------ (cipher1 + cipher2) ---------------
    cout << "\n------------------ SECOND OPERATION ------------------\n"
         << endl;
    // Compute (cipher1 + cipher2) for set 1
    cout << "Compute (cipher1 + cipher2) for set 1" << endl;

    // TIME START
    auto start_comp2_set1 = chrono::high_resolution_clock::now();

    evaluator.add(cipher_vec1_set1, cipher_vec2_set1, cipher_result2_set1);

    // TIME END
    auto stop_comp2_set1 = chrono::high_resolution_clock::now();
    auto duration_comp2_set1 = chrono::duration_cast<chrono::microseconds>(stop_comp2_set1 - start_comp2_set1);

    // Decrypt and Decode
    Plaintext plain_result2_set1;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result2_set1, plain_result2_set1);
    vector<double> vec_result2_set1;
    ckks_encoder.decode(plain_result2_set1, vec_result2_set1);
    print_vector(vec_result2_set1);

    cout << "\nTime to compute (cipher1 + cipher2): " << duration_comp2_set1.count() << " microseconds" << endl;

    // Compute (cipher1 + cipher2) for set 2
    cout << "Compute (cipher1 + cipher2) for set 2" << endl;

    // TIME START
    auto start_comp2_set2 = chrono::high_resolution_clock::now();

    evaluator.add(cipher_vec1_set2, cipher_vec2_set2, cipher_result2_set2);

    // TIME END
    auto stop_comp2_set2 = chrono::high_resolution_clock::now();
    auto duration_comp2_set2 = chrono::duration_cast<chrono::microseconds>(stop_comp2_set2 - start_comp2_set2);

    // Decrypt and Decode
    Plaintext plain_result2_set2;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result2_set2, plain_result2_set2);
    vector<double> vec_result2_set2;
    ckks_encoder.decode(plain_result2_set2, vec_result2_set2);
    print_vector(vec_result2_set2);

    cout << "\nTime to compute (cipher1 + cipher2): " << duration_comp2_set2.count() << " microseconds" << endl;

    // Compute (cipher1 + cipher2) for set 3
    cout << "Compute (cipher1 + cipher2) for set 3" << endl;

    // TIME START
    auto start_comp2_set3 = chrono::high_resolution_clock::now();

    evaluator.add(cipher_vec1_set3, cipher_vec2_set3, cipher_result2_set3);

    // TIME END
    auto stop_comp2_set3 = chrono::high_resolution_clock::now();
    auto duration_comp2_set3 = chrono::duration_cast<chrono::microseconds>(stop_comp2_set3 - start_comp2_set3);

    // Decrypt and Decode
    Plaintext plain_result2_set3;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result2_set3, plain_result2_set3);
    vector<double> vec_result2_set3;
    ckks_encoder.decode(plain_result2_set3, vec_result2_set3);
    print_vector(vec_result2_set3);

    cout << "\nTime to compute (cipher1 + cipher2): " << duration_comp2_set3.count() << " microseconds" << endl;

    cout << endl;

    // ------------------ (cipher1 * plain2) ---------------
    cout << "\n------------------ THIRD OPERATION ------------------\n"
         << endl;

    // Compute (cipher1 + plain2) for set 1
    cout << "Compute (cipher1 * plain2) for set 1" << endl;

    // TIME START
    auto start_comp3_set1 = chrono::high_resolution_clock::now();

    evaluator.multiply_plain(cipher_vec1_set1, plain_vec2_set1, cipher_result3_set1);

    // TIME END
    auto stop_comp3_set1 = chrono::high_resolution_clock::now();
    auto duration_comp3_set1 = chrono::duration_cast<chrono::microseconds>(stop_comp3_set1 - start_comp3_set1);

    // Decrypt and Decode
    Plaintext plain_result3_set1;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result3_set1, plain_result3_set1);
    vector<double> vec_result3_set1;
    ckks_encoder.decode(plain_result3_set1, vec_result3_set1);
    print_vector(vec_result3_set1);

    cout << "\nTime to compute (cipher1 * plain2): " << duration_comp3_set1.count() << " microseconds" << endl;

    // Compute (cipher1 * plain2) for set 2
    cout << "Compute (cipher1 * plain2) for set 2" << endl;

    // TIME START
    auto start_comp3_set2 = chrono::high_resolution_clock::now();

    evaluator.multiply_plain(cipher_vec1_set2, plain_vec2_set2, cipher_result3_set2);

    // TIME END
    auto stop_comp3_set2 = chrono::high_resolution_clock::now();
    auto duration_comp3_set2 = chrono::duration_cast<chrono::microseconds>(stop_comp3_set2 - start_comp3_set2);

    // Decrypt and Decode
    Plaintext plain_result3_set2;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result3_set2, plain_result3_set2);
    vector<double> vec_result3_set2;
    ckks_encoder.decode(plain_result3_set2, vec_result3_set2);
    print_vector(vec_result3_set2);

    cout << "\nTime to compute (cipher1 * plain2): " << duration_comp3_set2.count() << " microseconds" << endl;

    // Compute (cipher1 * plain2) for set 3
    cout << "Compute (cipher1 * plain2) for set 3" << endl;

    // TIME START
    auto start_comp3_set3 = chrono::high_resolution_clock::now();

    evaluator.multiply_plain(cipher_vec1_set3, plain_vec2_set3, cipher_result3_set3);

    // TIME END
    auto stop_comp3_set3 = chrono::high_resolution_clock::now();
    auto duration_comp3_set3 = chrono::duration_cast<chrono::microseconds>(stop_comp3_set3 - start_comp3_set3);

    // Decrypt and Decode
    Plaintext plain_result3_set3;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result3_set3, plain_result3_set3);
    vector<double> vec_result3_set3;
    ckks_encoder.decode(plain_result3_set3, vec_result3_set3);
    print_vector(vec_result3_set3);

    cout << "\nTime to compute (cipher1 * plain2): " << duration_comp3_set3.count() << " microseconds" << endl;

    cout << endl;

    // ------------------ (cipher1 * cipher2) ---------------
    cout << "\n------------------ FOURTH OPERATION ------------------\n"
         << endl;
    // Compute (cipher1 * cipher2) for set 1
    cout << "Compute (cipher1 * cipher2) for set 1" << endl;

    // TIME START
    auto start_comp4_set1 = chrono::high_resolution_clock::now();

    evaluator.multiply(cipher_vec1_set1, cipher_vec2_set1, cipher_result4_set1);

    // TIME END
    auto stop_comp4_set1 = chrono::high_resolution_clock::now();
    auto duration_comp4_set1 = chrono::duration_cast<chrono::microseconds>(stop_comp4_set1 - start_comp4_set1);

    // Decrypt and Decode
    Plaintext plain_result4_set1;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result4_set1, plain_result4_set1);
    vector<double> vec_result4_set1;
    ckks_encoder.decode(plain_result4_set1, vec_result4_set1);
    print_vector(vec_result4_set1);

    cout << "\nTime to compute (cipher1 * cipher2): " << duration_comp4_set1.count() << " microseconds" << endl;

    // Compute (cipher1 * cipher2) for set 2
    cout << "Compute (cipher1 * cipher2) for set 2" << endl;

    // TIME START
    auto start_comp4_set2 = chrono::high_resolution_clock::now();

    evaluator.multiply(cipher_vec1_set2, cipher_vec2_set2, cipher_result4_set2);

    // TIME END
    auto stop_comp4_set2 = chrono::high_resolution_clock::now();
    auto duration_comp4_set2 = chrono::duration_cast<chrono::microseconds>(stop_comp4_set2 - start_comp4_set2);

    // Decrypt and Decode
    Plaintext plain_result4_set2;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result4_set2, plain_result4_set2);
    vector<double> vec_result4_set2;
    ckks_encoder.decode(plain_result4_set2, vec_result4_set2);
    print_vector(vec_result4_set2);

    cout << "\nTime to compute (cipher1 * cipher2): " << duration_comp4_set2.count() << " microseconds" << endl;

    // Compute (cipher1 * cipher2) for set 3
    cout << "Compute (cipher1 * cipher2) for set 3" << endl;

    // TIME START
    auto start_comp4_set3 = chrono::high_resolution_clock::now();

    evaluator.multiply(cipher_vec1_set3, cipher_vec2_set3, cipher_result4_set3);

    // TIME END
    auto stop_comp4_set3 = chrono::high_resolution_clock::now();
    auto duration_comp4_set3 = chrono::duration_cast<chrono::microseconds>(stop_comp4_set3 - start_comp4_set3);

    // Decrypt and Decode
    Plaintext plain_result4_set3;
    cout << "Decrypt and decode the result" << endl;
    decryptor.decrypt(cipher_result4_set3, plain_result4_set3);
    vector<double> vec_result4_set3;
    ckks_encoder.decode(plain_result4_set3, vec_result4_set3);
    print_vector(vec_result4_set3);

    cout << "\nTime to compute (cipher1 * cipher2): " << duration_comp4_set3.count() << " microseconds" << endl;

    cout << endl;
}

int main()
{

    // Need to plot graph with x-axis as the size and y-axis as the duration
    // Plot 3 different graphs (with 3 different ploy_modulus_degree)
    EncryptionParameters params(scheme_type::CKKS);

    // Case 1 : poly_modulus_degree = 4096
    size_t poly_modulus_degree = 4096;
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

    // Run the tests
    ckksBenchmark(poly_modulus_degree);

    /*
    // Set output file
    string filename_1 = "bench_4096.dat";
    ofstream outf_1(filename_1);
    // Handle file error
    if (!outf_1)
    {
        cerr << "Couldn't open file: " << filename_1 << endl;
        exit(1);
    }

    // Write to file
    outf_1 << "2"
         << "\t\t"
         << "230" << endl;
    outf_1 << "3"
         << "\t\t"
         << "400" << endl;
    outf_1 << "5"
         << "\t\t"
         << "230" << endl;
    outf_1 << "6"
         << "\t\t"
         << "400" << endl;

    // Close the file
    outf_1.close();

    // Case 2 : poly_modulus_degree = 8192
    poly_modulus_degree = 8192;
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

    // Run the tests

    // Set output file
    string filename_2 = "bench_8192.dat";
    ofstream outf_2(filename_2);
    // Handle file error
    if (!outf_2)
    {
        cerr << "Couldn't open file: " << filename_2 << endl;
        exit(1);
    }

    // Write to file
    outf_2 << "2"
         << "\t\t"
         << "230" << endl;
    outf_2 << "3"
         << "\t\t"
         << "400" << endl;
    outf_2 << "5"
         << "\t\t"
         << "230" << endl;
    outf_2 << "6"
         << "\t\t"
         << "400" << endl;

    // Close the file
    outf_2.close();

    // Case 3 : poly_modulus_degree = 16384
    poly_modulus_degree = 16384;
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

    // Run the tests

    // Set output file
    string filename_3 = "bench_16384.dat";
    ofstream outf_3(filename_3);
    // Handle file error
    if (!outf_3)
    {
        cerr << "Couldn't open file: " << filename_3 << endl;
        exit(1);
    }

    // Write to file
    outf_3 << "2"
         << "\t\t"
         << "230" << endl;
    outf_3 << "3"
         << "\t\t"
         << "400" << endl;
    outf_3 << "5"
         << "\t\t"
         << "230" << endl;
    outf_3 << "6"
         << "\t\t"
         << "400" << endl;

    // Close the file
    outf_3.close();

*/
    return 0;
}