package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"

	"gopkg.in/src-d/go-billy.v4/memfs"
	"gopkg.in/src-d/go-git.v4"
	"gopkg.in/src-d/go-git.v4/storage/memory"
)

func downloadFmt(includeDirPath string) {
	outDir := filepath.Join(includeDirPath, "fmt")
	if _, err := os.Stat(outDir); err == nil {
		return
	}

	fs := memfs.New()

	_, err := git.Clone(memory.NewStorage(), fs, &git.CloneOptions{
		URL: "https://github.com/fmtlib/fmt",
	})
	if err != nil {
		panic(err)
	}

	files, _ := fs.ReadDir("include/fmt")
	os.Mkdir(outDir, 0777)
	for _, file := range files {
		in, _ := fs.Open(fmt.Sprintf("include/fmt/%s", file.Name()))
		defer in.Close()
		out, _ := os.Create(filepath.Join(outDir, file.Name()))
		defer out.Close()
		io.Copy(out, in)
	}
}
