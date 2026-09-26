---
category: "[[Engine Modules]]"
---
Final engine code, bringing together all subsystems.

# Frame Process
```mermaid
graph TD
	window --> ui --> renderPrepare --> ecs --> present
	
	window["Window::update()"]
	ui["Ui::update()"]
	renderPrepare["RenderPipeline::beginFrame()"]
	ecs["Ecs::update()"]
	present["RenderPipeline::present(mainCamera)"]
```
## `Window::update()`
Update window, read input, handle resize.
## `UI::update()`
Draw interface and handle input.
## `RenderPipeline::beginFrame()`
Prepare for rendering, update state to match window if changed.
## `Ecs::update()`
Main update process.
## `RenderPipeline::present(mainCamera)`
Present the main camera to the window.