#pragma once

#define SAFE_DELETE(object) delete(object); object = nullptr;