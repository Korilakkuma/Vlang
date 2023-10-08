/*---------------------------*/
/*  ‘f”‚ğ‹‚ß‚é cp_prime.c  */
/*---------------------------*/
int is_prime(int n)  /* n‚ª‘f”‚È‚ç^‚ğ•Ô‚· */
{
    int i;

    if (n < 2)    return 0;     /* 2–¢–‚Í‘f”‚Å‚È‚¢       */
    if (n == 2)   return 1;     /* 2‚Í‘f”‚Å‚ ‚é           */
    if (n%2 == 0) return 0;     /* 2ˆÈŠO‚Ì‹ô”‚Í‘f”‚Å‚È‚¢ */

    for (i=3; i*i<=n; i=i+2) {
        if (n%i == 0) return 0; /* Š„‚èØ‚ê‚½‚ç‘f”‚Å‚È‚¢ */
    }
    return 1;                   /* ‘f”‚Å‚ ‚é */
}

int main(void)
{
    int n;

    for (n=1; n<=1000; n++) {   /* 1000ˆÈ‰º‚Ì‘f”‚Í */
        if (is_prime(n)) {
            printf("%d ", n);
        }
    }
    printf("\n");

    return 0;
}

