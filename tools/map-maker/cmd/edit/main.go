package main

import (
	"encoding/json"
	"fmt"
	"image"
	"log"
	"math"
	"os"
	"path/filepath"
	"strings"

	"image/color"

	"github.com/hajimehoshi/ebiten/examples/resources/fonts"
	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
	"github.com/hajimehoshi/ebiten/v2/text"
	"github.com/nfnt/resize"
	"golang.org/x/image/font"
	"golang.org/x/image/font/opentype"

	"github.com/sardap/stop-the-mail/tools/map-maker/maker"

	_ "github.com/oov/psd"
)

var (
	mplusBigFont font.Face
)

func init() {
	tt, err := opentype.Parse(fonts.MPlus1pRegular_ttf)
	if err != nil {
		log.Fatal(err)
	}

	const dpi = 72
	mplusBigFont, err = opentype.NewFace(tt, &opentype.FaceOptions{
		Size:    16,
		DPI:     dpi,
		Hinting: font.HintingFull,
	})
	if err != nil {
		log.Fatal(err)
	}
}

type Mode int

const (
	ModeWaypoint Mode = iota
	ModePathWaiting
	ModePathDraging
	ModeStartingPostion
	ModeMoving
)

const (
	multiplier     = 1
	dsScreenWidth  = 256 * multiplier
	dsScreenHeight = 256 * multiplier
	screenWidth    = dsScreenWidth
	screenHeight   = 512
	pathSize       = 16 * multiplier
)

type Window struct {
	Image        *ebiten.Image
	ImagePath    string
	Waypoints    []maker.Waypoint
	Paths        []maker.Path
	Mode         Mode
	Rounds       []maker.Round
	OutPath      string
	SpawnPostion maker.Position
	startPosX    int
	startPosY    int
	selected     maker.Moveable
}

func (w *Window) Save() error {
	gen := maker.Generate{
		Waypoints:        w.Waypoints,
		StartingPosition: w.SpawnPostion,
		ImageFilename:    w.ImagePath,
		Rounds:           w.Rounds,
		Paths:            w.Paths,
	}

	bitmapName := strings.TrimSuffix(filepath.Base(w.ImagePath), filepath.Ext(w.ImagePath))
	data, _ := json.MarshalIndent(gen, "", "\t")
	os.WriteFile(filepath.Join(w.OutPath, bitmapName+".json"), data, 0777)

	return nil
}

func (w *Window) Update() error {

	if inpututil.IsKeyJustPressed(ebiten.Key1) {
		w.Mode = ModePathWaiting
	} else if inpututil.IsKeyJustPressed(ebiten.Key2) {
		w.Mode = ModeWaypoint
	} else if inpututil.IsKeyJustPressed(ebiten.Key3) {
		w.Mode = ModeStartingPostion
	}

	if inpututil.IsKeyJustPressed(ebiten.KeyS) {
		w.Save()
	}

	x, y := ebiten.CursorPosition()
	switch w.Mode {
	case ModeWaypoint:
		if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonLeft) {
			if r := maker.PointInPath(x, y, w.Paths); r != nil {
				distX := math.Abs(float64(r.X - x))
				distY := math.Abs(float64(r.Y - y))

				x, y := x, y

				if distX < distY {
					x = r.X
				} else {
					y = r.Y
				}

				w.Waypoints = append(w.Waypoints, maker.Waypoint{X: x, Y: y})
			}
		}
		if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonRight) {
			for i, waypoint := range w.Waypoints {
				if maker.PointInRect(x, y, maker.Rect{X: waypoint.X, Y: waypoint.Y, W: pathSize, H: pathSize}) {
					w.selected = &w.Waypoints[i]
					w.Mode = ModeMoving
					break
				}
			}
		}
		if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonMiddle) {
			for i, waypoint := range w.Waypoints {
				if maker.PointInRect(x, y, maker.Rect{X: waypoint.X, Y: waypoint.Y, W: pathSize, H: pathSize}) {
					w.Waypoints = append(w.Waypoints[:i], w.Waypoints[i+1:]...)
					break
				}
			}
		}
	case ModePathWaiting:
		if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonLeft) {
			w.Mode = ModePathDraging
			w.startPosX = x
			w.startPosY = y
		}
		if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonRight) {
			for i, path := range w.Paths {
				if maker.PointInRect(x, y, path.AsRect()) {
					w.selected = &w.Paths[i]
					w.Mode = ModeMoving
					break
				}
			}
		}
		if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonMiddle) {
			for i, path := range w.Paths {
				if maker.PointInRect(x, y, path.AsRect()) {
					w.Paths = append(w.Paths[:i], w.Paths[i+1:]...)
					break
				}
			}
		}
	case ModePathDraging:
		if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonLeft) {
			// endX, _ := w.startPosX-x, w.startPosY-y
			var (
				xStart int
				xEnd   int
				yStart int
				yEnd   int
			)
			if x > w.startPosX {
				xStart = w.startPosX
				xEnd = x
			} else {
				xStart = x
				xEnd = w.startPosX
			}

			if y > w.startPosY {
				yStart = w.startPosY
				yEnd = y
			} else {
				yStart = y
				yEnd = w.startPosY
			}

			width := xEnd - xStart
			height := yEnd - yStart

			if width > height {
				height = pathSize * multiplier
			} else if height > width {
				width = pathSize * multiplier
			}

			w.Paths = append(w.Paths, maker.Path{X: xStart, Y: yStart, Width: width, Height: height})
			w.Mode = ModePathWaiting
		}
	case ModeStartingPostion:
		w.selected = &w.SpawnPostion
		w.Mode = ModeMoving
	case ModeMoving:
		if ebiten.IsKeyPressed(ebiten.KeyShiftLeft) && ebiten.IsKeyPressed(ebiten.KeyA) || inpututil.IsKeyJustPressed(ebiten.KeyA) {
			w.selected.Move(-1*multiplier, 0)
		}
		if ebiten.IsKeyPressed(ebiten.KeyShiftLeft) && ebiten.IsKeyPressed(ebiten.KeyD) || inpututil.IsKeyJustPressed(ebiten.KeyD) {
			w.selected.Move(1*multiplier, 0)
		}
		if ebiten.IsKeyPressed(ebiten.KeyShiftLeft) && ebiten.IsKeyPressed(ebiten.KeyW) || inpututil.IsKeyJustPressed(ebiten.KeyW) {
			w.selected.Move(0, -1*multiplier)
		}
		if ebiten.IsKeyPressed(ebiten.KeyShiftLeft) && ebiten.IsKeyPressed(ebiten.KeyS) || inpututil.IsKeyJustPressed(ebiten.KeyS) {
			w.selected.Move(0, 1*multiplier)
		}
	}

	return nil
}

func (w *Window) Draw(screen *ebiten.Image) {
	screen.DrawImage(w.Image, &ebiten.DrawImageOptions{})

	// Render Paths
	for _, path := range w.Paths {
		ebitenutil.DrawRect(screen, float64(path.X), float64(path.Y), float64(path.Width), float64(path.Height), color.RGBA{R: 0xFF, G: 0xFF, A: 128})
	}

	// Render waypoints
	for i, waypoint := range w.Waypoints {
		ebitenutil.DrawRect(screen, float64(waypoint.X), float64(waypoint.Y), pathSize, pathSize, color.RGBA{R: 255, A: 255})
		{
			pendingText := fmt.Sprintf("%d", i)
			b := text.BoundString(mplusBigFont, pendingText)
			text.Draw(screen, pendingText, mplusBigFont, waypoint.X+b.Max.X/2, waypoint.Y+b.Min.Y*-1+2, color.Black)
		}
	}

	// Dragging
	x, y := ebiten.CursorPosition()
	if w.Mode == ModePathDraging {
		endX, endY := w.startPosX-x, w.startPosY-y
		ebitenutil.DrawRect(screen, float64(x), float64(y), float64(endX), float64(endY), color.RGBA{R: 0, G: 0, B: 0xFF, A: 128})
	}

	// Spawn Postion
	ebitenutil.DrawRect(screen, float64(w.SpawnPostion.X), float64(w.SpawnPostion.Y), pathSize, pathSize, color.RGBA{B: 0xFF, A: 0xFF})

	ebitenutil.DebugPrint(screen, fmt.Sprintf("X: %d, Y: %d", x, y))

}

func (w *Window) Layout(outsideWidth, outsideHeight int) (int, int) {
	return dsScreenWidth, dsScreenHeight
}

func main() {
	mapPic := os.Args[1]
	outPath := os.Args[2]

	var genrate maker.Generate
	if filepath.Ext(mapPic) == ".json" {
		jsonBytes, _ := os.ReadFile(mapPic)
		json.Unmarshal(jsonBytes, &genrate)
	} else {
		genrate.Waypoints = nil
		genrate.Rounds = nil
		genrate.Paths = nil
		genrate.ImageFilename = mapPic
	}

	// Open map pic
	img := func() image.Image {
		f, err := os.Open(filepath.Join(outPath, genrate.ImageFilename))
		if err != nil {
			log.Fatalf("Cannot open %s", genrate.ImageFilename)
		}
		defer f.Close()

		result, _, err := image.Decode(f)
		if err != nil {
			log.Fatalf("Unable to decode image %s %v", genrate.ImageFilename, err)
		}

		result = resize.Resize(dsScreenWidth, dsScreenHeight, result, resize.NearestNeighbor)

		return result
	}()

	w := &Window{
		Image:        ebiten.NewImageFromImage(img),
		ImagePath:    genrate.ImageFilename,
		OutPath:      outPath,
		Waypoints:    genrate.Waypoints,
		Rounds:       genrate.Rounds,
		Paths:        genrate.Paths,
		SpawnPostion: genrate.StartingPosition,
	}

	ebiten.SetWindowSize(screenWidth, screenHeight)
	ebiten.SetWindowTitle("Stop the mail map maker")
	if err := ebiten.RunGame(w); err != nil {
		log.Fatal(err)
	}

}
