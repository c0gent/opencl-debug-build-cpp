#include <iostream>
#include <thread>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

const int TH_C = 20;
const int LOOP_C = 200;

static const char source[] =
	"kernel void add(\n"
    "       __global const float *a,\n"
    "       __global const float *b,\n"
    "       __global float *c)\n"
    "{\n"
    "    size_t i = get_global_id(0);\n"
    "    c[i] = a[i] + b[i];\n"
    "}\n";


// Compile program in a loop.
void buildloop(cl::Device device, cl::Context context) {
	std::vector<std::thread> threads;

	for (int t_i = 0; t_i < TH_C; t_i++) {
		threads.push_back(std::thread ([](cl::Device device, cl::Context context) { 
			for (int i = 0; i < LOOP_C; i++) {
				cl::Program::Sources sources(1, std::make_pair(source, strlen(source)));
				cl::Program program(context, sources);

				try {
				    program.build("-cl-std=CL1.1 -DCL_CONFIG_CPU_VECTORIZER_MODE=1");
				} catch (const cl::Error&) {
				    std::cerr
						<< "OpenCL compilation error" << std::endl
						<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
						<< std::endl;
				    return;
				}
			}
		}, device, context));
		std::cout << "        Thread " << t_i << " spawned. "
			<< "Running " << LOOP_C << " iterations."
			<< std::endl;
	}

	for (auto t = threads.begin(); t != threads.end(); t++) {
        t->join();
    }

	return;
}

int main() {
	try {
		int check_count = 0;

		// Iterate through platforms:
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		if (platforms.empty()) {
			std::cerr << "No platforms found." << std::endl;
			exit(1);
		}

		// for (auto p = platforms.begin(); p != platforms.end(); p++) {
		for (auto p = platforms.end() - 1; p >= platforms.begin(); p--) {
			std::cout << "Platform: " << p->getInfo<CL_PLATFORM_NAME>() << std::endl;
			// Iterate through devices:
			std::vector<cl::Device> devices;
			p->getDevices(CL_DEVICE_TYPE_ALL, &devices);

			for (auto d = devices.begin(); d != devices.end(); d++) {
				// Skip unavailable devices:
				if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

				std::cout << "    Checking device: " 
					<< d->getInfo<CL_DEVICE_NAME>()
					<< std::endl;
				cl::Context context = cl::Context(*d);
				buildloop(*d, context);
				check_count += 1;
			}
		}

		std::cout << "\nDevices checked: " << check_count << std::endl;
	} catch (const cl::Error &err) {
		std::cerr
		    << "OpenCL error: "
		    << err.what() << "(" << err.err() << ")"
		    << std::endl;
		return 1;
	}
}