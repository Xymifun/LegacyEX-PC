local root = vim.fn.getcwd()

vim.lsp.config("clangd", {
    cmd = {
        "./clangd-docker.sh",
        root,
        "pc64",
    },

    root_dir = root,

    filetypes = { "c", "cpp", "objc", "objcpp" },

    init_options = {
        fallbackFlags = { "-std=c++20" },
    },
})

vim.lsp.enable("clangd")
