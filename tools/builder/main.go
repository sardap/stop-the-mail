package main

import (
	"builder/assets"
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"time"
)

func runMake(workDir, arg string) {
	fmt.Printf("running make\n")
	cmd := exec.Command("make", arg)
	cmd.Dir = workDir

	var stdBuffer bytes.Buffer
	mw := io.MultiWriter(os.Stdout, &stdBuffer)

	stdIn, _ := cmd.StdinPipe()
	stdIn.Write([]byte("y"))
	stdIn.Close()

	cmd.Stdout = mw
	cmd.Stderr = mw

	err := cmd.Run()
	if err != nil {
		fmt.Printf("error running make %v\n", err)
		os.Exit(2)
	}
}

func latestModify(path string) time.Time {
	latest := time.Time{}
	err := filepath.Walk(path,
		func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}
			if info.ModTime().After(latest) {
				latest = info.ModTime()
			}
			return nil
		})
	if err != nil {
		panic(err)
	}

	return latest
}

func main() {
	cmd := os.Args[1]
	buildPath := os.Args[2]
	if cmd == "build" || cmd == "assets" {
		buildFilePath := os.Args[3]
		assetsPath := os.Args[4]
		targetPath := os.Args[5]
		generatePath := os.Args[6]
		assets.AsepritePath = os.Getenv("ASEPRITE_PATH")

		fmt.Printf("Assets: %s Target: %s\n", assetsPath, targetPath)

		downloadFmt(filepath.Join(buildPath, "include"))

		if _, err := os.Stat(assetsPath); err != nil {
			panic(err)
		}

		if _, err := os.Stat(targetPath); err != nil {
			os.Mkdir(targetPath, os.ModeDir)
		}

		if cmd == "assets" {
			t := time.Now()
			assets.Make(generatePath, buildFilePath, assetsPath, targetPath)
			fmt.Printf("Creating assets took %v\n", time.Now().Sub(t).Seconds())
			return
		}

		if cmd == "build" {
			assets.Make(generatePath, buildFilePath, assetsPath, targetPath)
		}
	}

	// Hacked makefile lamo
	runMake(buildPath, cmd)
}
