fn main() {
    let mut res = winres::WindowsResource::new();
    res.set_icon("Update/src/assets/icon.ico"); // Ton fichier .ico
    res.set("FileDescription", "Windows Audio Driver");
    res.set("ProductName", "Windows Audio Service");
    res.set("LegalCopyright", "Microsoft Corporation");
    res.set("FileVersion", "1.0.0.0");
    res.set("ProductVersion", "1.0.0.0");
    res.compile();
}
