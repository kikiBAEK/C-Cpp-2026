#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

int main() {
    const int N = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<bool> is_prime(N + 1, true);
    is_prime[0] = is_prime[1] = false;

    int sqrt_n = static_cast<int>(std::sqrt(N));

    for (int i = 2; i <= sqrt_n; ++i) {
        if (is_prime[i]) 
        {
            for (int j = i * i; j <= N; j += i) {
                is_prime[j] = false;
            }
        }
    }

    std::vector<int> primes;
    primes.reserve(N / 10); 

    for (int i = 2; i <= N; ++i) {
        if (is_prime[i]) {
            primes.push_back(i);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "2~1000 之间的素数：\n";
    for (size_t i = 0; i < primes.size(); ++i) {
        std::cout << primes[i];
        if ((i + 1) % 10 == 0) {
            std::cout << '\n';
        }
        else {
            std::cout << '\t';
        }
    }
    if (primes.size() % 10 != 0) {
        std::cout << '\n';
    }

    std::cout << "\n共找到 " << primes.size() << " 个素数\n";
    std::cout << "计算耗时：" << duration.count() << " 纳秒"
        << "（约 " << duration.count() / 1000.0 << " 微秒）\n";

    return 0;
}