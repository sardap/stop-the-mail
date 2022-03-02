package main

import (
	"bytes"
	"fmt"
	"image"
	"image/png"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"time"
)

type Generate struct {
	Waypoints        []Waypoint
	Rounds           []Round
	Paths            []Path
	StartingPosition Position
	ImageFilename    string
}

const tempPngFile = "temp.png"
const tempCFile = "temp.c"
const tempHFile = "temp.h"

func psdToPng(path string) {
	inF, err := os.Open(path)
	if err != nil {
		log.Fatalf("Cannot open %s", path)
	}
	defer inF.Close()

	img, _, err := image.Decode(inF)
	if err != nil {
		log.Fatalf("Unable to decode image %s %v", path, err)
	}

	outF, _ := os.Create(tempPngFile)
	defer outF.Close()

	png.Encode(outF, img)
}

func (g *Generate) generateGrit(assetsPath string) error {
	wd, _ := os.Getwd()
	cmd := exec.Command("grit", []string{
		filepath.Join(wd, tempPngFile),
		"-W3",
		"-gT!",  // No Transparent colour
		"-gzl",  // llZ Compression
		"-gB16", // bit depth 16
		"-gb",   // Bitmap
		"-ftc",  // Create C file out
	}...)

	os.Remove(tempPngFile)
	psdToPng(filepath.Join(assetsPath, g.ImageFilename))
	defer os.Remove(tempPngFile)

	var stdBuffer bytes.Buffer
	cmd.Stdout = &stdBuffer
	cmd.Stderr = &stdBuffer
	result := cmd.Run()
	return result
}

func getBitmapValues(fileContains string) []string {
	re := regexp.MustCompile(`0[xX][0-9a-fA-F]+`)

	return re.FindAllString(fileContains, -1)
}

func (g *Generate) Generate(assetsPath, outPath string) {

	if err := g.generateGrit(assetsPath); err != nil {
		panic(err)
	}

	bitmapName := strings.TrimSuffix(filepath.Base(g.ImageFilename), filepath.Ext(g.ImageFilename))
	bitmapVarName := bitmapName + "Bitmap"

	cFile, _ := os.ReadFile("temp.c")
	os.Remove(tempCFile)
	os.Remove(tempHFile)
	bitmapValues := getBitmapValues(string(cFile))

	builder := &strings.Builder{}

	// Header File
	builder.WriteString("/*\n AUTO GENERATED FILE DO NOT EDIT\n")
	builder.WriteString("\tGENERATED on " + time.Now().Format("2006-01-02 15:04:05") + "\n*/\n")
	builder.WriteString("#pragma once\n\n")
	builder.WriteString("#include<defence_scene.hpp>\n\n")

	builder.WriteString(fmt.Sprintf("extern const unsigned int "+bitmapVarName+"[%d];\n\n", len(bitmapValues)))

	builder.WriteString("extern const sm::level::Level " + bitmapName + "Level;\n")

	os.MkdirAll(filepath.Join(outPath, "generated"), 0777)

	os.WriteFile(filepath.Join(outPath, "generated", bitmapName+".hpp"), []byte(builder.String()), 0777)

	// C File
	builder.Reset()

	builder.WriteString("/*\n AUTO GENERATED FILE DO NOT EDIT\n")
	builder.WriteString("\tGENERATED on " + time.Now().Format("2006-01-02 15:04:05") + "\n*/\n")
	builder.WriteString(fmt.Sprintf("#include <generated/%s.hpp>\n\n", bitmapName))

	builder.WriteString(fmt.Sprintf("const unsigned int "+bitmapVarName+"[%d] __attribute__((aligned(4))) = {\n\t", len(bitmapValues)))
	for i, val := range bitmapValues {
		builder.WriteString(val)
		if i < len(bitmapValues)-1 {
			builder.WriteString(", ")
		}
		if i > 0 && i%6 == 0 {
			builder.WriteString("\n\t")
		}
	}
	builder.WriteString("\n};\n\n")

	builder.WriteString("const sm::level::Level " + bitmapName + "Level = {\n")
	builder.WriteString("\t.mail_waypoints = sm::Waypoints({\n")
	for _, waypoint := range g.Waypoints {
		builder.WriteString(fmt.Sprintf("\t\tsm::Position{sm::Fixed(%d), sm::Fixed(%d)},\n", waypoint.X, waypoint.Y))
	}
	builder.WriteString("\t}),\n")
	builder.WriteString(fmt.Sprintf("\t.starting_point = sm::Position{sm::Fixed(%d), sm::Fixed(%d)},\n", g.StartingPosition.X, g.StartingPosition.Y))

	// Rounds
	builder.WriteString("\t.rounds = {\n")
	for _, round := range g.Rounds {
		builder.WriteString("\t\tsm::level::Round{\n")
		builder.WriteString("\t\t\t.spawns = {\n")
		for _, spawn := range round.Spawns {
			builder.WriteString("\t\t\t\tsm::level::SpawnInfo{")
			builder.WriteString(fmt.Sprintf(".frame = %d, ", spawn.Frame))
			builder.WriteString(fmt.Sprintf(".type = static_cast<sm::level::MailType>(%d) ", spawn.MailType))
			builder.WriteString("},\n")
		}
		builder.WriteString("\t\t\t},\n")
		builder.WriteString("\t\t},\n")

	}
	builder.WriteString("\t},\n")

	builder.WriteString(fmt.Sprintf("\t.background = sm::level::Background{.bitmap=%s},\n", bitmapVarName))
	builder.WriteString("};\n")

	os.WriteFile(filepath.Join(outPath, "generated", bitmapName+".cpp"), []byte(builder.String()), 0777)
}
