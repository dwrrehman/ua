#include "test.hpp"
#include <math.h>
#include <string.h>
void _() {
    int m = 5, n = 1, l = pow(m, n), r[l], s[l];
    memset(r, 0, sizeof r); 
    r[0] = 1;
    for (int i = l * m; i--;) {
        memcpy(s, r, sizeof s);
        for (int j = l; j--;) {            
            for (int k = l; k--;) 
                r[j] += s[k] * ((j + k) * (j + k));  // still a work in progress.            
            r[j] = (r[j] + !!r[j]) % m;   // more sbr required here.
        }
    }
}




















//// obfuscated version:







#include"test.hpp"
#include<math.h>
#include<string.h>
void o(){int m=5,n=1,l=pow(m,n),r[l],s[l];memset(r,0,sizeof r);r[0]=1;
for(int i=l*m;i--;){memcpy(s,r,sizeof s);for(int j=l;j--;){
for(int k=l;k--;){r[j]+=s[k]*((j+k)*(j+k));}r[j]=(r[j]+!!r[j])%m;}}}








