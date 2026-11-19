# OpenGL Game Project

Cenário 3D desenvolvido em C++ com OpenGL durante a disciplina de Computação Gráfica, colocando em prática os conteúdos das aulas.

## Funcionalidades

- Cenário com casas, árvores, postes, Fusca, Neymar e Pikachu.
- Modelos 3D, texturas, iluminação e transformações.
- Câmera com movimento pelo teclado e mouse e detecção de colisões.
- Animação de nuvens, partículas de chuva e névoa.
- Reutilização dos comandos de desenho com Display Lists.
- Inicialização em tela cheia.

## Controles

| Tecla | Ação |
| --- | --- |
| W, A, S, D | Movimentar a câmera |
| Mouse | Olhar ao redor |
| F | Ligar ou desligar as luzes dos postes |
| Espaço | Restaurar a câmera inicial |
| Esc | Fechar o jogo |

## Tecnologias

C++17, OpenGL, GLFW, GLEW, Assimp e stb_image.

## Executar

Com as dependências instaladas e os caminhos do compilador ajustados em `.vscode`, abra a pasta no VS Code e pressione F5. Mantenha as pastas `models` e `textures` junto ao executável.

O código principal está em `main.cpp`, e o carregamento dos modelos está em `glb_model.h`. Os créditos dos modelos estão em `models/ASSET_CREDITS.txt` e `models/trees/CREDITS.txt`.
