# OpenGL Game Project

Projeto de estudos da disciplina de Computacao Grafica, desenvolvido em C++ com OpenGL.

O repositorio acompanha a evolucao das atividades da disciplina e servira como base para um jogo 3D simples no futuro.

## Cena atual

- Carro esportivo em modelo 3D GLB, estacionado junto a calcada.
- Rua com textura de asfalto e faixas desenhadas em OpenGL.
- Calcadas elevadas, meio-fio e gramados criados com geometria e cores.
- Casa em modelo GLB, apoiada no gramado e com caminho ate a calcada.
- Seis postes em modelo OBJ, tres de cada lado da rua, com luzes locais.
- Um cone de transito em modelo GLB.
- Fusca estacionado no gramado ao lado da casa; carro esportivo na calcada oposta.
- Neymar em pose estatica segurando um guarda-chuva preto e Pikachu ao seu lado.
- Nuvens em movimento e chuva animadas em funcao do tempo (Aula 13).
- Camera controlada por teclado (`W`, `A`, `S`, `D`) e mouse.
- Carro esportivo fixo no cenario, com posicao, tamanho e colisao constantes.
- Display Lists para reutilizar os comandos de desenho dos modelos.

## Controles de iluminacao e animacao

- `W`, `A`, `S`, `D`: caminham pelo cenario.
- Mouse: controla a direcao do olhar.
- `F`: liga ou desliga as luzes dos seis postes.
- Espaco: restaura a posicao e a direcao iniciais da camera.
- `Esc`: fecha o jogo.

Os postes permanecem fixos. A iluminacao ambiente global fica fixa em 0.45, mantendo a cena clara com brilho suavizado.

## Camera e colisoes - Aula 15

Procure por `AULA 15 - CAMERA E COLISOES` em `main.cpp`. As configuracoes e funcoes destas duas partes ficam juntas nesse bloco. As chamadas de inicializacao, atualizacao e desenho ficam no fluxo principal do programa.

### Roteiro de apresentacao: siga as etapas 1 a 7

As etapas abaixo seguem a mesma ordem dos comentarios numerados de `main.cpp`. Procure pelo titulo de cada etapa com Ctrl+F; os numeros de linha servem como referencia desta versao e podem mudar em futuras edicoes.

### 1. Configuracoes - linha 137

Definem velocidade, sensibilidade, altura, tamanho do jogador, limites e enquadramento inicial.

| Configuracao | Valor atual | Funcao |
| --- | --- | --- |
| `kCameraSpeed` | `9.0f` | Velocidade em unidades do cenario por segundo |
| `kMouseSensitivity` | `0.1f` | Sensibilidade do olhar pelo mouse |
| `kCameraEyeHeight` | `3.0f` | Altura dos olhos acima do piso |
| `kCameraRadius` | `0.25f` | Raio usado para representar o jogador nas colisoes |
| `kCameraPitchLimit` | `85.0f` | Limite de inclinacao vertical em graus |
| `kCameraMinimumX/Z`, `kCameraMaximumX/Z` | Limites do terreno, com margem para o raio | Impedem sair do cenario |

A camera inicia na rua em X = -2 e Z = -22.9, olhando para Neymar e Pikachu em primeiro plano e para o Fusca a esquerda. O angulo horizontal (`yaw`) inicial e 152.5 graus e o vertical (`pitch`) e 0 graus. Os valores de posicao e altura sao unidades do cenario, sem conversao definida para metros.

### 2. Altura do piso - linha 161

`camera_ground_height()` recebe a posicao X/Z, identifica se ela pertence a rua, calcada, caminho da casa ou gramado e devolve a altura daquela superficie. A funcao considera `kWorldScale`, que amplia o cenario.

A altura dos olhos e sempre a altura do piso somada a `kCameraEyeHeight`. Isso permite acompanhar os desniveis sem entrar no chao. A mudanca de altura e direta, sem simulacao de gravidade, pulo ou suavizacao de degraus.

### 3. Olhar com o mouse - linha 171

`mouse_read()` calcula quanto o mouse se moveu desde a leitura anterior. O deslocamento horizontal altera `yaw`; o vertical altera `pitch`, ambos considerando a sensibilidade configurada.

`std::clamp()` limita o pitch entre -85 e +85 graus, evitando virar a visao completamente. A primeira leitura apenas registra a posicao do mouse para evitar um salto inicial.

`update_camera_direction()` converte os angulos em uma direcao X/Y/Z usando seno e cosseno. Essa direcao e usada pelo `gluLookAt()` no desenho da cena.

### 4. Restaurar a camera - linha 193

`reset_camera()` restaura a posicao X/Z e os angulos iniciais, calcula a altura correta acima do piso e chama `update_camera_direction()` para atualizar o olhar. E usada na inicializacao e quando o jogador pressiona Espaco.

### 5. Caminhada - linha 203

`update_camera()` le WASD e calcula o movimento a partir do angulo horizontal da camera. W/S avancam ou recuam; A/D deslocam para os lados. A caminhada modifica apenas X/Z, por isso olhar para cima ou para baixo nao faz a camera voar.

O deslocamento e calculado por velocidade multiplicada pelo tempo entre quadros (`dt`). Esse intervalo e limitado a 0.1 segundo para evitar saltos grandes depois de uma pausa. A direcao e normalizada para que andar na diagonal nao seja mais rapido.

O movimento e dividido em passos pequenos. Cada passo fica dentro dos limites do terreno e so e aceito se `camera_position_is_free()` indicar que nao existe colisao. X e Z sao testados separadamente, permitindo deslizar pela lateral dos obstaculos. Ao final, a altura e atualizada com:

```cpp
camera_eye_y = camera_ground_height(camera_eye_x, camera_eye_z) + kCameraEyeHeight;
```

A declaracao antecipada de `camera_position_is_free()` avisa ao C++ que a funcao existe. Sua implementacao fica na etapa 7 para manter este roteiro de leitura.

### 6. Preparar colisoes - linha 234

`CollisionBox` representa uma caixa invisivel com seis limites: `min_x`, `min_y`, `min_z`, `max_x`, `max_y` e `max_z`.

`model_collision_bounds()` percorre os vertices normalizados do modelo e encontra seus menores e maiores valores em cada eixo. `transform_collision_box()` aplica escala, rotacao e posicao para levar essa caixa ate a posicao do objeto no mundo, incluindo `kWorldScale`.

`initialize_collisions()` registra as caixas da casa, Fusca, Neymar, Pikachu, cone, postes e carro esportivo. Nos postes, os vertices da base localizam a haste; o braco elevado fica livre para passar. Nuvens e guarda-chuva nao recebem caixas de colisao.

As caixas sao calculadas uma vez, apos carregar os modelos e definir suas alturas. O carro esportivo permanece fixo, com posicao, tamanho e colisao constantes; os antigos controles T, R e E foram removidos.

### 7. Testar colisoes - linha 324

`camera_hits_box()` primeiro verifica se existe sobreposicao vertical entre o obstaculo e o corpo do jogador, considerado entre o piso e a altura dos olhos. Objetos inteiramente acima ou abaixo desse intervalo nao bloqueiam a passagem.

Depois, considera o jogador como um circulo visto de cima. `std::clamp()` encontra o ponto da caixa mais proximo da posicao X/Z da camera. Se a distancia ao quadrado ate esse ponto for menor que `kCameraRadius * kCameraRadius`, existe colisao. A comparacao ao quadrado evita calcular uma raiz quadrada.

`camera_position_is_free()` percorre todas as caixas. Retorna `false` assim que encontra uma colisao e `true` quando nenhuma bloqueia a posicao. A caminhada da etapa 5 consulta esse resultado antes de aceitar cada movimento.

As caixas sao aproximacoes retangulares: nao acompanham cada detalhe das superficies e, principalmente em objetos girados, podem bloquear um pouco antes da parte visivel.

### Retoques finais da cena

Os ajustes de finalizacao incluem o enquadramento inicial do grupo, a iluminacao clara fixa com brilho suavizado e a retirada dos controles e indicadores de transformacao do carro. A iluminacao usa o modelo por vertices do OpenGL, sem sombras projetadas.

Os modelos 3D usam Display Lists para reutilizar o desenho. Camera e colisoes sao processadas fora dessas listas. Nas nuvens, o deslocamento e aplicado antes de desenhar o modelo; a chuva e desenhada a cada quadro porque suas particulas mudam de posicao.

A chuva e as nuvens permanecem animadas continuamente, sem controles para desligar ou pausar. O ceu mantem o tom azulado de tempo chuvoso.

## Animacao da Aula 13

A funcao `animate()` usa `glfwGetTime()` para calcular o tempo decorrido entre quadros (`delta_time`). As posicoes sao atualizadas por velocidade multiplicada por esse intervalo, seguindo a estrutura do exemplo da disciplina.

As nuvens se deslocam horizontalmente e retornam ao outro lado ao sair da area. A chuva usa 1.600 particulas, distribuidas entre cinco nuvens (320 por nuvem). Cada gota guarda o indice da sua nuvem e deslocamentos relativos em X e Z. Sua posicao horizontal e calculada somando esses deslocamentos a posicao atual da nuvem, acompanhando seu movimento.

A altura Y diminui conforme a velocidade de queda e o delta de tempo. Ao chegar ao chao, a gota e reciclada abaixo da mesma nuvem, dentro de uma regiao oval proporcional ao tamanho do modelo. As particulas sao desenhadas diretamente como linhas (`GL_LINES`); os modelos das nuvens usam Display Lists reutilizadas com transformacoes de posicao.

## Texturas e geometria

A imagem `textures/aerial_asphalt_01_diff_1k.jpg` e aplicada a superficie da rua por coordenadas UV. A imagem permanece inteira e se repete sobre o asfalto. As calcadas e os gramados ficam ao lado da pista, em superficies separadas.

As calcadas usam quadrilateros cinza elevados, com laterais para o meio-fio e pequenas juntas entre as placas. Os gramados usam quadrilateros com variacoes de verde. As faixas da pista e o caminho da casa tambem sao geometria colorida, sem imagens de textura. O carro esportivo, a casa e o Neymar usam texturas dos arquivos GLB; os demais modelos usam cores de materiais. O Pikachu baixado nao incluia o material referenciado, por isso recebeu materiais de cores aproximadas.

## Tecnologias

- C++17
- OpenGL
- GLFW
- GLEW
- Assimp
- stb_image

## Estrutura

- `main.cpp`: inicializacao, controles, camera e desenho da cena.
- `glb_model.h`: carregamento e desenho dos modelos 3D.
- `models/`: modelos da cena e arquivo `ASSET_CREDITS.txt` com os creditos dos novos modelos.
- `textures/`: imagem usada no plano da rua.

## Observacao

Este projeto tem finalidade educacional e esta sendo desenvolvido gradualmente durante as aulas de Computacao Grafica.
