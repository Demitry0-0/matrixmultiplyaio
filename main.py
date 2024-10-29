import asyncio

import matrixmultiplyaio


def matrix_multiply_py(a, b):
    row_size = len(a)
    col_size = len(b[0])

    result = [[0.] * col_size for _ in range(row_size)]

    size = len(b)
    for i in range(row_size):
        for j in range(col_size):
            for k in range(size):
                result[i][j] += a[i][k] * b[k][j]
    return result


def matrix_multiply_c(a: tuple[tuple[float, ...], ...], b: tuple[tuple[float, ...], ...]):
    return matrixmultiplyaio.matrix_multiply(a, b)


def matrix_multiply_c_thread(a: tuple[tuple[float, ...], ...], b: tuple[tuple[float, ...], ...]):
    loop = asyncio.get_running_loop()
    return matrixmultiplyaio.matrix_multiply_async(a, b, loop)


async def main():
    res = [matrix_multiply_c_thread(tuple(tuple(range(i)) for _ in range(i)),
                                    tuple(tuple(range(i)) for _ in range(i)))
           for i in range(3, 5)]
    res = await asyncio.gather(*res)
    print(res)

if __name__ == '__main__':
    asyncio.run(main())
