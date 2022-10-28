with open('codigo.txt') as f:
    dados = f.readlines()

for linha in range(len(dados)):
    dados[linha] = dados[linha][1:]

with open('codigo.txt', 'w') as f:
    for i in dados:
        f.write(i)