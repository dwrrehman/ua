int main() {
    const unsigned long long m = 2, n = 1, s = 100, S = 100, t = 1000;
    unsigned long long g[S], h[S];
    for (unsigned long long i = 0; i < S; i++) g[i] = 0;
    g[0] = 1;
    for (unsigned long long i = 0; i < t; i++) {
        for (unsigned long long j = 0; j < S; j++) h[j] = g[j];
        for (unsigned long long j = 0; j < S; j++)
            g[j] = (g[j] + h[(j + 1) % s] *
                    (h[j] * (h[(j + s - 1) % s] + 1) + 1)) % m;
    }
    return 0;
}
