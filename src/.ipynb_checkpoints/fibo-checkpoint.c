//#include<bits/stdc++.h>
//
//#define eps 1e-6
//#define ls (rt << 1)
//#define rs (rt << 1 | 1)
//#define lowbit( x ) (x&(-x))
//#define SZ( v ) ((int)(v).size())
//#define All( v ) (v).begin(), (v).end()
//#define mp( x , y ) make_pair(x,y)
//#define fast ios::sync_with_stdio(0), cin.tie(0), cout.tie(0)
//using namespace std;
//typedef long long ll;
//typedef unsigned long long ull;
//typedef pair < int , int > P;
//const int N = 1e5 + 10;
//const int M = 1e7 + 10;
//const int mod = 1e9 + 7;
//const int inf = 0x3f3f3f3f;
//const int INF = 2e9;
//const int seed = 131;
//int n , k ,dp[N][2];
//char s[N];
//int main () {
//    scanf("%s",s+1);
//    n = strlen(s+1);
//    dp[0][0] = 0 , dp[0][1] = 1;
//    for(int i = 1; i <= n; ++ i){
//        dp[i][0] = min(dp[i - 1][0] + s[i] - '0' , dp[i - 1][1] + 10 - (s[i] - '0') ) ;
//        dp[i][1] = min(dp[i - 1][0] + s[i] - '0'+ 1 , dp[i - 1][1] + 10 - (s[i] - '0' ) - 1 );
//    }
//    printf("%d\n",dp[n][0]);
//	return 0;
//}
#include <stdio.h>

int fibo (int n)
{
        int a=1,b=1,sum;
        int i;
        if (n == 1)
            printf("1\n");
        else if (n==2)
            printf("1 1\n");
        else
        {
                printf("1 1");
                for (i=3;i<=n;)
                {
                        sum=a+b;
                        b=a;
                        a=sum;
                        printf(" %d",sum);
                        i++;
                }
                puts("");
        }
        return 0;
}

int main()
{
        int var;
        scanf("%d",&var);
        fibo(var);
        return 0;
}
