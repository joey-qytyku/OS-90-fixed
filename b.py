import urllib
import urllib.request
import re

url = '''https://docs.google.com/document/d/e/2PACX-1vQGUck9HIFCyezsrBSnmENk5ieJuYwpt7YHYEzeNJkIb9OSDdx-ov2nRNReKQyey-cwJOoEKUhLmN9z/pub'''

urllib.request.urlretrieve(url, "pub")


def main(url:str):
    f = open("pub")

    data = f.read()

    # The cX number is not deterministic, but is the only instance of
    # That class for a span. No other classed span appears to exist.
    result = re.search(r'''<span class="c[0-9]">.+<\/span>''', data)

    # Each group of 3 is a row.

    # Delete anything still in brackets and leave only the values
    # as tokens.
    tokens = re.split('<.+?>', result.group())

    # Null strings are "Falsy" and filter removes false items with None lambda
    tokens = list(filter(None, tokens))

    # Clean out the table headers
    del tokens[0:4]

    # Convert the numbers to integers.
    for i, x in enumerate(tokens):
        if tokens[i].isnumeric():
            tokens[i] = int(x)

    # Get largest number for save width and height for matrix.
    dim = max([x for x in tokens if isinstance(x, int)])+1

    # Can't just use multplication for outer because it creates references
    # rather than deep copies.
    matrix = [['_']*dim for i in range(dim)]

    # Now start filling it out

    while len(tokens) != 0:
        y = tokens.pop()
        c = tokens.pop()
        x = tokens.pop()
        print(f"{x} {c} {y}")
        # Plot the character
        matrix[y][x] = c

    # Print results
    for u in reversed(matrix):
        for v in u:
            print(v, end="")
        print("\n")

main(url)
