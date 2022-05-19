package assets

import (
	"builder/gbacolour"
	"encoding/gob"
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"log"

	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/ettle/strcase"
	"github.com/pbnjay/pixfont"

	_ "github.com/oov/psd"

	"github.com/pelletier/go-toml"
)

var (
	AsepritePath string
	// Grit Really doens't like async shit
	gritMutex = &sync.Mutex{}
)

const (
	ModePhotoshop = ""
	ModeAseprite  = "aseprite"
)

type GraphicsCache struct {
	BuildFileMod time.Time
	ModTimes     map[string]time.Time
}

func (g *GraphicsCache) Update(assetPath string, gfx *GraphicsOutput) {
	for _, f := range gfx.Files {
		stat, _ := os.Stat(filepath.Join(assetPath, f))
		g.ModTimes[f] = stat.ModTime()
	}
}

type GraphicsOutput struct {
	Name    string
	Mode    string
	Options []string
	Files   []string
	Dirs    []string
}

func (g *GraphicsOutput) clearTargetFiles(targetPath string) {
	//Clear shared files
	for _, o := range g.Options {
		if strings.Contains(o, "-O ") {
			sharedFilename := strings.Split(o, " ")[1]
			os.Remove(filepath.Join(
				targetPath, fmt.Sprintf("%s.h", sharedFilename)),
			)
			os.Remove(filepath.Join(
				targetPath, fmt.Sprintf("%s.c", sharedFilename)),
			)
		}
	}

	for _, filename := range g.Files {
		filename := filepath.Base(filename)
		os.Remove(filepath.Join(targetPath, fmt.Sprintf("%s.h", filename)))
		os.Remove(filepath.Join(targetPath, fmt.Sprintf("%s.c", filename)))
	}
}

func (g *GraphicsOutput) Changed(gfxCache *GraphicsCache) bool {
	for _, f := range g.Files {
		lastMod, ok := gfxCache.ModTimes[f]
		if ok || time.Now().After(lastMod) {
			return true
		}
	}

	return false
}

func buildMapMaker(mapGenPath string) string {
	path, err := exec.LookPath(mapGenPath)
	if err == nil {
		return path
	}

	cmd := exec.Command("go", "build", "-o", "map-maker.exe", ".")
	cmd.Dir = mapGenPath

	var stdBuffer bytes.Buffer
	mw := io.MultiWriter(os.Stdout, &stdBuffer)
	cmd.Stdout = mw
	cmd.Stderr = mw

	cmd.Run()

	return filepath.Join(mapGenPath, "map-maker.exe")
}

type MapOutput struct {
	Path string
}

func (m *MapOutput) Generate(mapGenPath, assetsPath, targetPath string) {
	if mapStat, err := os.Stat(mapGenPath); err == nil {
		genFile := filepath.Join(targetPath, "generated", filepath.Base(strings.TrimSuffix(m.Path, filepath.Ext(m.Path))+".cpp"))
		targetStat, err := os.Stat(genFile)
		if err == nil && targetStat.ModTime().Before(mapStat.ModTime()) {
			fmt.Printf("Map not changed %s\n", m.Path)
			return
		}

	}

	cmd := exec.Command(mapGenPath, targetPath, assetsPath, filepath.Join(assetsPath, m.Path))

	var stdBuffer bytes.Buffer
	mw := io.MultiWriter(os.Stdout, &stdBuffer)
	cmd.Stdout = mw
	cmd.Stderr = mw

	fmt.Printf("Building Map %s\n", m.Path)
	err := cmd.Run()
	if err != nil {
		log.Fatalf("Error building map %s %v", m.Path, err)
	}
}

const gfxCacheFile = "gfxCache.gob"

func loadGfxCache() *GraphicsCache {
	f, err := os.Open(gfxCacheFile)
	if err != nil {
		return &GraphicsCache{
			ModTimes:     make(map[string]time.Time),
			BuildFileMod: time.Time{},
		}
	}
	defer f.Close()

	var result GraphicsCache
	dec := gob.NewDecoder(f)
	dec.Decode(&result)

	return &result
}

func saveGfxCache(gfxCache *GraphicsCache) {
	f, _ := os.Create(gfxCacheFile)
	defer f.Close()
	enc := gob.NewEncoder(f)
	enc.Encode(*gfxCache)
}

func Make(generatePath, buildFilePath, assetsPath, targetPath string) {
	gfxTargetPath := filepath.Join(targetPath, "gfx")

	os.MkdirAll(gfxTargetPath, 0777)

	buildFile, err := os.ReadFile(buildFilePath)
	if err != nil {
		panic(err)
	}

	var config struct {
		Graphics []GraphicsOutput
		Maps     []MapOutput
	}
	if err := toml.Unmarshal(buildFile, &config); err != nil {
		panic(err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(60)*time.Second)
	defer cancel()

	fmt.Printf("\nRunning Graphics\n")

	//Change into dir for grit
	workingDir, _ := os.Getwd()
	os.Chdir(gfxTargetPath)
	defer os.Chdir(workingDir)

	gfxCache := loadGfxCache()
	defer saveGfxCache(gfxCache)

	buildFileStat, _ := os.Stat(buildFilePath)

	for _, target := range config.Graphics {

		if !gfxCache.BuildFileMod.Before(buildFileStat.ModTime()) && target.Changed(gfxCache) {
			fmt.Printf("\nNo Changes %s\n", target.Name)
			continue
		}

		fmt.Printf("\nBuilding %s\n", target.Name)

		target.clearTargetFiles(targetPath)

		err := gritCall(ctx, assetsPath, targetPath, &target)
		if err != nil && !strings.Contains(err.Error(), "signal: segmentation fault") {
			fmt.Printf("Grit error Probably doesn't matter %v\n", err)
		}

		gfxCache.Update(assetsPath, &target)
	}

	gfxCache.BuildFileMod = buildFileStat.ModTime()

	fmt.Printf("\nRunning Maps\n")

	mapMakerPath := buildMapMaker(generatePath)
	fmt.Printf("Map maker built %s\n", mapMakerPath)

	for _, target := range config.Maps {
		target.Generate(mapMakerPath, assetsPath, targetPath)
	}
}

func gritCall(ctx context.Context, assetsPath, targetPath string, target *GraphicsOutput) error {
	gritMutex.Lock()
	defer gritMutex.Unlock()

	var arguments []string
	for _, f := range target.Files {
		pngFile := genPng(context.Background(), filepath.Join(assetsPath, f))
		defer os.Remove(pngFile)
		arguments = append(arguments, pngFile)
	}
	for _, option := range target.Options {
		arguments = append(arguments, strings.Split(option, " ")...)
	}

	fmt.Printf("Calling grit %v\n", arguments)
	cmd := exec.CommandContext(ctx, "grit", arguments...)

	var stdBuffer bytes.Buffer
	mw := io.MultiWriter(os.Stdout, &stdBuffer)
	cmd.Stdout = mw
	cmd.Stderr = mw

	return cmd.Run()
}

func replaceTransparentColour(filename string) {
	transColour := color.RGBA{R: 255, G: 0, B: 255, A: 255}
	blankColour := color.RGBA{}

	f, _ := os.Open(filename)
	img, _, _ := image.Decode(f)
	rgbaImg := img.(draw.Image)
	f.Close()
	for y := 0; y < rgbaImg.Bounds().Dy(); y++ {
		for x := 0; x < rgbaImg.Bounds().Dx(); x++ {
			if rgbaImg.At(x, y) == blankColour {
				rgbaImg.Set(x, y, transColour)
			}
		}
	}
}

func psdToPng(ctx context.Context, filename string, outfile string) {
	ch := make(chan struct{})

	go func() {
		file, err := os.Open(filename)
		if err != nil {
			panic(err)
		}
		defer file.Close()

		img, _, err := image.Decode(file)
		if err != nil {
			panic(err)
		}

		if strings.Contains(filename, "tsTitleText") {
			img = overlayVersion(img)
		}

		img = gbacolour.ConvertImg(img)

		pngFile, _ := os.Create(outfile)
		defer pngFile.Close()
		png.Encode(pngFile, img)

		close(ch)
	}()

	select {
	case <-ctx.Done():
		if ctx.Err() != nil {
			panic(ctx.Err())
		}
	case <-ch:
	}
}

func asepriteToPng(ctx context.Context, filename, outfile string) {
	cmd := exec.CommandContext(ctx, AsepritePath, filename, "--batch", "--sheet", outfile)
	if err := cmd.Run(); err != nil {
		panic(err)
	}
}

func getVersion() string {
	cmd := exec.Command("git", "describe", "--tags", "--abbrev=0")
	out, err := cmd.CombinedOutput()
	if err != nil {
		tag := os.Getenv("GIT_TAG")
		if tag == "" {
			fmt.Printf("out %s and env empty", out)
			os.Exit(2)
		}

		return tag
	}

	return string(out)
}

func overlayVersion(inImg image.Image) image.Image {
	textImg := image.NewRGBA(image.Rect(0, 0, 64, 16))

	pixfont.DrawString(textImg, 0, 0, getVersion(), color.White)

	for y := 0; y < textImg.Rect.Max.Y; y++ {
		for x := 0; x < textImg.Rect.Max.X; x++ {
			if textImg.RGBAAt(x, y).A == 0 {
				textImg.Set(x, y, color.RGBA{R: 255, G: 0, B: 246, A: 255})
			}
		}
	}

	b := inImg.Bounds()
	dst := image.NewRGBA(image.Rect(0, 0, b.Dx(), b.Dy()))
	draw.Draw(dst, dst.Bounds(), inImg, b.Min, draw.Src)
	draw.Draw(dst, textImg.Bounds().Add(image.Point{X: 0, Y: 160 - 8}), textImg, image.Point{}, draw.Over)

	return dst
}

func genPng(ctx context.Context, inputFile string) string {

	idx := strings.Index(inputFile, string(filepath.Separator)+"gfx")
	outfile, _ := os.Getwd()
	outfile += string(filepath.Separator)
	for _, part := range strings.Split(inputFile[idx+4:], string(filepath.Separator)) {
		outfile += strcase.ToPascal(strings.TrimSuffix(part, filepath.Ext(part)))
	}
	outfile += ".png"

	switch filepath.Ext(inputFile) {
	case ".psd":
		psdToPng(ctx, inputFile, outfile)
	case ".aseprite":
		asepriteToPng(ctx, inputFile, outfile)
	default:
		panic(fmt.Errorf("unknown ext %s", filepath.Ext(inputFile)))
	}

	replaceTransparentColour(outfile)

	return outfile
}
