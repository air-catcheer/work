with open('case_database.txt.enc', 'r') as file:
    main = file.read()

plain = []
start = 0

while start < 1000:
    if start > 863:
        start = 1
        break
    else:
        plain.append(main[start])
        start += 32

start = 1

for i in range(1, 63):
    for j in range(1, 27):
        if start > 864:
            start = start - 864 + 1
            break
        plain.append(main[start])
        start += 32

print("".join(plain))
