/*---------------------------*/
/*  �f�������߂� cp_prime.c  */
/*---------------------------*/
int is_prime(int n)  /* n���f���Ȃ�^��Ԃ� */
{
    int i;

    if (n < 2)    return 0;     /* 2�����͑f���łȂ�       */
    if (n == 2)   return 1;     /* 2�͑f���ł���           */
    if (n%2 == 0) return 0;     /* 2�ȊO�̋����͑f���łȂ� */

    for (i=3; i*i<=n; i=i+2) {
        if (n%i == 0) return 0; /* ����؂ꂽ��f���łȂ� */
    }
    return 1;                   /* �f���ł��� */
}

int main(void)
{
    int n;

    for (n=1; n<=1000; n++) {   /* 1000�ȉ��̑f���� */
        if (is_prime(n)) {
            printf("%d ", n);
        }
    }
    printf("\n");

    return 0;
}

