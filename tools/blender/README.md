# Blender plugins

## io_scene_lite3d

Плагин для экспорта обьектов сцены в формате Lite3d из Blender. Экспортируются такие обьекты как:
- Иерархия обьектов (scene/)
- Обьекты сцены (objects/)
- Узлы сцены:
    - Mesh Nodes
    - Источники света Light Nodes
- Материалы (materials/)
- Текстуры (textures/)
- Каркасы (models/)

Шейдеры, камеры, эелементы пайплайна не экспортируются и отстаюся на усмотрение разработчика.

### Install

Для установки скопировать папку io_scene_lite3d в директорию add-on Blender. Посмотреть путь можно в настройках. 
Включить плагин в настройках Add-on, категория Import-Export.

### How to Use

Для экспорта нажать File -> Export -> Lite3d Scene Export. Экспортируется вся текущая сцена. Выбрать отдельный обьект для 
экспорта нельзя. 
При экспорте можно выбрать несколько опций. Опции разделены по группам.  
![](/tools/blender/params.png "Options")

#### Package Name

Имя пакета. Будет автоматически вставляться во все пути. 

#### Save Tangents

По умолчанию в layout каркаса сохраняются вершины, нормали, текстурные кординаты(только нулевой слой). Опция позволяет дополнительно 
сохранить тангенты.

#### Save BiTangents

Опция позволяет дополнительно сохранить битангенты в layout каркаса.

#### Remove Doubles

Опцтимизация каркаса. Обьединение близко лежащих вершин. Использовать аккуратно так как применяется напрямую к обьектам в сцене до 
их сохранения. Можно сделать вручную из меню Blender.

#### Triangulate

Преобразование граней квадов в треугольники. Плагин умеет работать только с треугольными гранями в силу ограничений движка. 
Применятеся ко всем обьектам сцены до сохранения. Можно сделать вручную из меню Blender. 

#### Copy Texture Images

Попытаться скопировать текстуры в целевую папку. ВАЖНО! Для корректной работы нужно использовать абсолютные пути при загрузке текстур.
Если текстуры зашиты в Blender файл, то попытается их выгрузить.

#### Material template name

Шаблон по умолчанию для всех текстур (должен лежать в materials/). Используется для сохранения материалов. В шаблоне присутствует ряд
подстановок из Shader Node BSDF_PRINCIPLED, в случае если к ним не пристыкованы другие ноды.
- **Albedo** Базовый цвет
- **Emission** Излучающий свет
- **Metallic** Степень металичности материала
- **Specular** Кофф отраженния материала
- **Roughness** Кофф шероховатости материала
- **IOR** Индекс преломления света 

Так же вместе с этим все Image Nodes (TEX_IMAGE) материала будут рассматриваться в качестве подстановок по label. 
Например есть в шаблоне есть подстановка **Albedo** и есть Image Node c label=Albedo тогда в шаблон включится текстура из 
этой ноды.

#### Default Attenuation Constant

Константное затухание света по умолчанию. Может быть задано для каждого источника света в Custom Properties, 
опция **AttenuationConstant**  
![](/tools/blender/custom_props.png "Custom Properties")

#### Default Attenuation Linear

Линейное затухание от расстояния до источника света по умолчанию. Может быть задано для каждого источника света в Custom Properties, 
опция **AttenuationLinear**

#### Default Attenuation Quadratic

Квадратичное затухание от расстояния до источника света по умолчанию. Может быть задано для каждого источника света в Custom Properties, 
опция **AttenuationQuadratic**

#### Default Influence Distance

Расстоние на которое влияет свет источника света по умолчанию. Может быть задано для каждого источника света в Custom Properties, 
опция **InfluenceDistance**

#### Default Influence Minimum Radiance

Минимальное излучение света влияющее не обьекты по умолчанию. Может быть задано для каждого источника света в Custom Properties, 
опция **InfluenceMinRadiance**

### Objects and nodes

Сцена движка оперирует обьектами, шаблоны обьектов описываются в json и лежат в папке objects/. В сцену можно добавлять не 
ограниченное количество обьектов по шаблону, но все они должны иметь уникальные имена. Обьект имеет свою позицию и ориентацию,
и подключается к сцене в json файле описания сцены которые лежат в папке scenes/. Обьекты состоят из узлов и образуют 
иерархическую структуру схожую с обьектами в blender. Каждый узел так же имеет свою позицию и ориентацию относительно родителя.
К узлам могут быть привязаны Mesh и Light и одновременно он может быть родителем других узлов. Вложенность не ограничена.  

Каждый верхнеуровневый обьект в blender сохраняется как шаблон обьекта. Если обьект blender имеет parent то он сохраняется как 
узел родитеского обьекта. Сцена может оперировать множеством одинаковых обьектов, отличающихся только ориентацией, в этом случае 
логично было бы один раз сохранить шаблон обьекта и его переиспользовать. Для этого предусмотрена custom опция **originObject**,
задается на вкладке обьекта и содежит название того обьекта шаблон которого мы хотим переиспользовать.   
![](/tools/blender/origin_prop.png "originObject")